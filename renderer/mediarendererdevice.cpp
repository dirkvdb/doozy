//    Copyright (C) 2012 Dirk Vanden Boer <dirk.vdb@gmail.com>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include "typeconversions.h"
#include "upnp/upnp.protocolinfo.h"

#include "mediarendererdevice.h"

#include "utils/log.h"
#include "utils/backtrace.h"
#include "utils/stringoperations.h"
#include "utils/fileoperations.h"
#include "utils/readerfactory.h"

#include "audio/audioplaybackfactory.h"
#include "upnp/upnp.utils.h"
#include "upnp/upnp.http.reader.h"

#include "audioconfig.h"
#include "devicedescriptions.h"

#ifdef HAVE_LIBCEC
    #include "ceccontrol.h"
#endif

#include <sstream>
#include <limits>

using namespace utils;
using namespace upnp;
using namespace audio;
using namespace std::placeholders;
using namespace std::chrono_literals;

namespace doozy
{

namespace
{

std::string toString(const std::set<PlaybackAction>& actions)
{
    std::stringstream ss;
    for (auto& action : actions)
    {
        if (ss.tellp() > 0) ss << ',';

        ss << action;
    }

    return ss.str();
}

AVTransport::State PlaybackStateToTransportState(PlaybackState state)
{
    switch (state)
    {
    case PlaybackState::Playing:        return AVTransport::State::Playing;
    case PlaybackState::Paused:         return AVTransport::State::PausedPlayback;
    case PlaybackState::Stopped:
    default:                            return AVTransport::State::Stopped;
    }
}

#ifdef HAVE_LIBCEC

const uint32_t g_idleTimeout = 5;

void TurnOnCecDevice(const std::string& dev)
{
    try
    {
        CecControl cec(dev);
        cec.turnOn();
        if (!cec.isActiveSource())
        {
            cec.setActiveSource();
        }
    }
    catch (const std::runtime_error& e)
    {
        log::warn(e.what());
    }
}

void TurnOffCecDevice(const std::string& dev)
{
    try
    {
        log::info("Turn off receiver, idle for {} minutes", g_idleTimeout);
        CecControl cec(dev);
        if (cec.isActiveSource())
        {
            cec.standBy();
        }
    }
    catch (const std::runtime_error& e)
    {
        log::warn(e.what());
    }
}
#endif

}

MediaRendererDevice::MediaRendererDevice(RendererSettings& settings)
: m_settings(settings)
, m_playback(PlaybackFactory::create("Custom", "Doozy", m_settings.getAudioOutput(), m_settings.getAudioDevice(), m_queue))
, m_rootDevice(180s, m_io)
, m_connectionManager(m_rootDevice, *this)
, m_renderingControl(m_rootDevice, *this)
, m_avTransport(m_rootDevice, *this)
, m_cecDevice(m_settings.getCecDevice())
{
    // make sure we can read http urls
    ReaderFactory::registerBuilder(std::make_unique<upnp::http::ReaderBuilder>());

    m_playback->PlaybackStateChanged.connect([this] (PlaybackState state) {
        setTransportVariable(0, AVTransport::Variable::TransportState, AVTransport::Service::toString(PlaybackStateToTransportState(state)));
        CheckCecState(state);
    }, this);

    m_playback->AvailableActionsChanged.connect([this] (const std::set<PlaybackAction>& actions) {
        setTransportVariable(0, AVTransport::Variable::CurrentTransportActions, toString(actions));
    }, this);

    m_playback->ProgressChanged.connect([this] (double progress) {
        std::chrono::seconds secs(static_cast<uint32_t>(progress));
        setTransportVariable(0, AVTransport::Variable::RelativeTimePosition, durationToString(secs));
    }, this);

    m_playback->NewTrackStarted.connect([this] (const std::shared_ptr<ITrack>& track) {
        auto item = std::dynamic_pointer_cast<PlayQueueItem>(track);
        assert(item);

        addAlbumArtToWebServer(item);
        std::chrono::seconds duration(static_cast<uint32_t>(m_playback->getDuration()));

        setTransportVariable(0, AVTransport::Variable::CurrentTrackURI,         item->getUri());
        setTransportVariable(0, AVTransport::Variable::CurrentTrackMetaData,    item->getMetadataString());
        setTransportVariable(0, AVTransport::Variable::AVTransportURI,          item->getAVTransportUri());
        setTransportVariable(0, AVTransport::Variable::NextAVTransportURI,      m_queue.getNextUri());
        setTransportVariable(0, AVTransport::Variable::CurrentTrackDuration,    durationToString(duration));
        setTransportVariable(0, AVTransport::Variable::NumberOfTracks,          std::to_string(m_queue.getNumberOfTracks()));
    }, this);
}

MediaRendererDevice::~MediaRendererDevice() = default;

void MediaRendererDevice::signalHandler(const boost::system::error_code& error, int signo)
{
    if (!error)
    {
        if (signo == SIGSEGV)
        {
            log::critical("Segmentation fault");
            utils::printBackTrace();
            exit(EXIT_FAILURE);
        }
        else
        {
            log::info("Termination requested, stop renderer");
            stop();
        }
    }
}

void MediaRendererDevice::start(const std::string& networkInterface)
{
    try
    {
        m_thread.start();

        m_rootDevice.ControlActionRequested = std::bind(&MediaRendererDevice::onControlActionRequest, this, _1);
        m_rootDevice.EventSubscriptionRequested = std::bind(&MediaRendererDevice::onEventSubscriptionRequest, this, _1);

        if (networkInterface.empty())
        {
            m_rootDevice.initialize();
        }
        else
        {
            m_rootDevice.initialize("en0");
        }

        auto webroot = m_rootDevice.getWebrootUrl();

        Device deviceInfo;
        deviceInfo.type         = DeviceType(DeviceType::MediaRenderer, 1);
        deviceInfo.udn          = "uuid:" + m_settings.getUdn();
        deviceInfo.friendlyName = m_settings.getFriendlyName();
        deviceInfo.location     = "/rootdesc.xml";
        auto description        = fmt::format(s_mediaRendererDevice, deviceInfo.friendlyName, deviceInfo.udn, webroot);

        log::info("FriendlyName = {}", deviceInfo.friendlyName);
        log::info("AudioOutput = {}", m_settings.getAudioOutput());
        log::info("AudioDevice = {}", m_settings.getAudioDevice());

        m_rootDevice.addFileToHttpServer("/RenderingControlDesc.xml", "text/xml", s_rendererControlService);
        m_rootDevice.addFileToHttpServer("/ConnectionManagerDesc.xml", "text/xml", s_connectionManagerService);
        m_rootDevice.addFileToHttpServer("/AVTransportDesc.xml", "text/xml", s_avTransportService);

        Service rc;
        rc.type = ServiceType(ServiceType::RenderingControl, 1);
        deviceInfo.services.emplace(rc.type.type, rc);

        Service av;
        av.type = ServiceType(ServiceType::AVTransport, 1);
        deviceInfo.services.emplace(av.type.type, av);

        m_rootDevice.registerDevice(description, deviceInfo);

        setInitialValues();

        asio::signal_set signals(m_io, SIGINT, SIGTERM, SIGSEGV);
        signals.async_wait([this] (const auto& error, auto signo) { this->signalHandler(error, signo); });

        m_io.run();
        log::debug("Renderer ready");
    }
    catch(std::exception& e)
    {
        log::error(e.what());
    }
}

void MediaRendererDevice::stop()
{
    m_thread.stop();

    m_rootDevice.ControlActionRequested = nullptr;
    m_rootDevice.EventSubscriptionRequested = nullptr;

    m_rootDevice.uninitialize();
}

void MediaRendererDevice::setInitialValues()
{
#if defined(HAVE_MAD) || defined(HAVE_FFMPEG)
    m_supportedProtocols.push_back(ProtocolInfo("http-get:*:audio/mpeg:DLNA.ORG_PN=MP3"));
#endif

#if defined(HAVE_FLAC) || defined(HAVE_FFMPEG)
    m_supportedProtocols.push_back(ProtocolInfo("http-get:*:audio/flac:*"));
    m_supportedProtocols.push_back(ProtocolInfo("http-get:*:audio/x-flac:*"));
#endif

#ifdef HAVE_FFMPEG // assume ffmpeg supports these formats (possibly make this more smart and actually check ffmpeg config options)
    m_supportedProtocols.push_back(ProtocolInfo("http-get:*:audio/L16;rate=44100;channels=1:DLNA.ORG_PN=LPCM"));
    m_supportedProtocols.push_back(ProtocolInfo("http-get:*:audio/L16;rate=44100;channels=2:DLNA.ORG_PN=LPCM"));
    //m_supportedProtocols.push_back(ProtocolInfo("http-get:*:audio/L16;rate=48000;channels=1:DLNA.ORG_PN=LPCM"));
    //m_supportedProtocols.push_back(ProtocolInfo("http-get:*:audio/L16;rate=48000;channels=2:DLNA.ORG_PN=LPCM"));
    m_supportedProtocols.push_back(ProtocolInfo("http-get:*:audio/x-ms-wma:DLNA.ORG_PN=WMABASE"));
    m_supportedProtocols.push_back(ProtocolInfo("http-get:*:audio/x-ms-wma:DLNA.ORG_PN=WMAFULL"));
    m_supportedProtocols.push_back(ProtocolInfo("http-get:*:audio/mp4:DLNA.ORG_PN=AAC_ISO"));
    m_supportedProtocols.push_back(ProtocolInfo("http-get:*:audio/3gpp:DLNA.ORG_PN=AAC_ISO"));
    //m_supportedProtocols.push_back(ProtocolInfo("http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=AAC_ADTS_320"));
    m_supportedProtocols.push_back(ProtocolInfo("http-wavetunes:*:audio/x-ms-wma:*"));
    m_supportedProtocols.push_back(ProtocolInfo("http-get:*:audio/wav:*"));
    m_supportedProtocols.push_back(ProtocolInfo("http-get:*:audio/x-wav:*"));
#endif

    m_connectionManager.setVariable(ConnectionManager::Variable::SourceProtocolInfo, "");
    m_connectionManager.setVariable(ConnectionManager::Variable::SinkProtocolInfo, str::join(m_supportedProtocols, ","));
    m_connectionManager.setVariable(ConnectionManager::Variable::CurrentConnectionIds, "0");
    m_connectionManager.setVariable(ConnectionManager::Variable::ArgumentTypeConnectionStatus, "OK");

    m_currentConnectionInfo.connectionStatus    = ConnectionManager::ConnectionStatus::Ok;
    m_currentConnectionInfo.direction           = ConnectionManager::Direction::Input;

    m_renderingControl.setVariable(RenderingControl::Variable::PresetNameList, "FactoryDefaults");
    m_renderingControl.setVolume(0, RenderingControl::Channel::Master, m_playback->getVolume());
    m_renderingControl.setMute(0, RenderingControl::Channel::Master, m_playback->getMute());

    m_avTransport.setInstanceVariable(0, AVTransport::Variable::TransportState, AVTransport::Service::toString(PlaybackStateToTransportState(m_playback->getState())));
    m_avTransport.setInstanceVariable(0, AVTransport::Variable::TransportStatus, AVTransport::Service::toString(AVTransport::Status::Ok));
    m_avTransport.setInstanceVariable(0, AVTransport::Variable::PlaybackStorageMedium, "NETWORK");
    m_avTransport.setInstanceVariable(0, AVTransport::Variable::RecordStorageMedium, "NOT_IMPLEMENTED");
    m_avTransport.setInstanceVariable(0, AVTransport::Variable::PossiblePlaybackStorageMedia, "NONE,NETWORK");
    m_avTransport.setInstanceVariable(0, AVTransport::Variable::PossibleRecordStorageMedia, "NOT_IMPLEMENTED");
    m_avTransport.setInstanceVariable(0, AVTransport::Variable::TransportPlaySpeed, "1");
    m_avTransport.setInstanceVariable(0, AVTransport::Variable::CurrentPlayMode, AVTransport::Service::toString(AVTransport::PlayMode::Normal));
    m_avTransport.setInstanceVariable(0, AVTransport::Variable::TransportPlaySpeed, "1");
    m_avTransport.setInstanceVariable(0, AVTransport::Variable::RecordMediumWriteStatus, "NOT_IMPLEMENTED");
    m_avTransport.setInstanceVariable(0, AVTransport::Variable::CurrentRecordQualityMode, "NOT_IMPLEMENTED");
    m_avTransport.setInstanceVariable(0, AVTransport::Variable::PossibleRecordQualityModes, "NOT_IMPLEMENTED");
    m_avTransport.setInstanceVariable(0, AVTransport::Variable::NumberOfTracks, std::to_string(m_queue.getNumberOfTracks()));
    m_avTransport.setInstanceVariable(0, AVTransport::Variable::CurrentTrack, "0");
    m_avTransport.setInstanceVariable(0, AVTransport::Variable::CurrentTrackDuration, durationToString(0s));
    m_avTransport.setInstanceVariable(0, AVTransport::Variable::CurrentMediaDuration, "NOT_IMPLEMENTEDº");
    m_avTransport.setInstanceVariable(0, AVTransport::Variable::CurrentTrackMetaData, "");
    m_avTransport.setInstanceVariable(0, AVTransport::Variable::CurrentTrackURI, "");
    m_avTransport.setInstanceVariable(0, AVTransport::Variable::AVTransportURI, "");
    m_avTransport.setInstanceVariable(0, AVTransport::Variable::AVTransportURIMetaData, "");
    m_avTransport.setInstanceVariable(0, AVTransport::Variable::NextAVTransportURI, "");
    m_avTransport.setInstanceVariable(0, AVTransport::Variable::NextAVTransportURIMetaData, "");
    m_avTransport.setInstanceVariable(0, AVTransport::Variable::RelativeTimePosition, durationToString(0s));
    m_avTransport.setInstanceVariable(0, AVTransport::Variable::AbsoluteTimePosition, "NOT_IMPLEMENTED");
    m_avTransport.setInstanceVariable(0, AVTransport::Variable::RelativeCounterPosition, std::to_string(std::numeric_limits<int32_t>::max()));
    m_avTransport.setInstanceVariable(0, AVTransport::Variable::AbsoluteTimePosition, std::to_string(std::numeric_limits<int32_t>::max()));
    m_avTransport.setInstanceVariable(0, AVTransport::Variable::CurrentTransportActions, toString(m_playback->getAvailableActions()));
}

void MediaRendererDevice::setTransportVariable(uint32_t instanceId, AVTransport::Variable var, const std::string& value)
{
    // TODO: avoid copies
    m_io.post([=] () {
        m_avTransport.setInstanceVariable(instanceId, var, value);
    });
}

upnp::DeviceSubscriptionResponse MediaRendererDevice::onEventSubscriptionRequest(const upnp::SubscriptionRequest& request)
{
    log::debug("Renderer: event subscription request {} {}", request.sid, request.url);

    upnp::DeviceSubscriptionResponse response;
    response.timeout = request.timeout;

    if (request.url == "/AVTransport/evt")
    {
        response.initialEvent = m_avTransport.getSubscriptionResponse();
    }
    else if (request.url == "/RenderingControl/evt")
    {
        response.initialEvent = m_renderingControl.getSubscriptionResponse();
    }
    else if (request.url == "/ConnectionManager/evt")
    {
        response.initialEvent = m_connectionManager.getSubscriptionResponse();
    }
    else
    {
        log::warn("Invalid event subscription request: {}", request.sid);
    }

    return response;
}

std::string MediaRendererDevice::onControlActionRequest(const upnp::ActionRequest& request)
{
    log::debug("Renderer: action request: {}", request.actionName);

    //log::debug(request.action);

    switch (serviceTypeUrnStringToService(request.serviceType).type)
    {
    case ServiceType::AVTransport:
        return m_avTransport.onAction(request.actionName, request.action).toString();
    case ServiceType::RenderingControl:
        return m_renderingControl.onAction(request.actionName, request.action).toString();
    case ServiceType::ConnectionManager:
        return m_connectionManager.onAction(request.actionName, request.action).toString();
    default:
        throw InvalidSubscriptionId();
    }
}

bool MediaRendererDevice::supportsProtocol(const ProtocolInfo& info) const
{
    for (auto& protocol : m_supportedProtocols)
    {
        if (protocol.isCompatibleWith(info))
        {
            return true;
        }
    }

    return false;
}

void MediaRendererDevice::addAlbumArtToWebServer(const PlayQueueItemPtr& item)
{
    auto thumb = item->getAlbumArtThumb();
    if (!thumb.empty())
    {
        m_rootDevice.removeFileFromHttpServer("/Doozy/albumartthumb.jpg");
        m_rootDevice.addFileToHttpServer("/Doozy/albumartthumb.jpg", "image/jpeg", thumb);
        item->setAlbumArtUri(m_rootDevice.getWebrootUrl() + "/Doozy/albumartthumb.jpg", upnp::dlna::ProfileId::JpegThumbnail);
    }

    auto art = item->getAlbumArt();
    if (!art.empty())
    {
        m_rootDevice.removeFileFromHttpServer("/Doozy/albumart.jpg");
        m_rootDevice.addFileToHttpServer("/Doozy/albumart.jpg", "image/jpeg", art);
        item->setAlbumArtUri(m_rootDevice.getWebrootUrl() + "/Doozy/albumart.jpg", upnp::dlna::ProfileId::JpegLarge);
    }
}

void MediaRendererDevice::throwOnBadInstanceId(uint32_t id) const
{
    if (id != 0)
    {
        throw AVTransport::InvalidInstanceId();
    }
}

/***************************************************
 * Rendering control calls
 ***************************************************/

void MediaRendererDevice::selectPreset(uint32_t instanceId, const std::string& /*name*/)
{
    throwOnBadInstanceId(instanceId);
}

void MediaRendererDevice::setVolume(uint32_t instanceId, RenderingControl::Channel channel, uint16_t value)
{
    throwOnBadInstanceId(instanceId);

    if (value > 100 || channel != RenderingControl::Channel::Master)
    {
        throw InvalidArguments();
    }

    m_playback->setVolume(value);
    m_renderingControl.setVolume(instanceId, channel, m_playback->getVolume());
}

void MediaRendererDevice::setMute(uint32_t instanceId, RenderingControl::Channel channel, bool enabled)
{
    m_playback->setMute(enabled);
    m_renderingControl.setMute(instanceId, channel, m_playback->getMute());
}


/***************************************************
 * Connection Manager calls
 ***************************************************/

void MediaRendererDevice::prepareForConnection(const ProtocolInfo& protocolInfo, ConnectionManager::ConnectionInfo& info)
{
    if (info.direction != ConnectionManager::Direction::Input)
    {
        throw ConnectionManager::IncompatibleDirections();
    }

    if (!supportsProtocol(protocolInfo))
    {
        throw ConnectionManager::IncompatibleProtocol();
    }

    // currently we only support one instance
    info.connectionId = 0;
    info.avTransportId = 0;
    info.renderingControlServiceId = 0;
}

void MediaRendererDevice::connectionComplete(int32_t /*connectionId*/)
{
    // No actions necessary because instances are not supported yet
}

upnp::ConnectionManager::ConnectionInfo MediaRendererDevice::getCurrentConnectionInfo(int32_t connectionId)
{
    if (connectionId != 0)
    {
        throw ConnectionManager::InvalidConnectionReference();
    }

    return m_currentConnectionInfo;
}

/***************************************************
 * AVTransport calls
 ***************************************************/

void MediaRendererDevice::setAVTransportURI(uint32_t instanceId, const std::string& uri, const std::string& metaData)
{
    throwOnBadInstanceId(instanceId);

    try
    {
        log::info("Play uri ({}): {}", instanceId, uri);
        m_queue.setCurrentUri(uri);
        if (m_playback->isPlaying())
        {
            log::info("Stop and play");
            m_playback->stop();
            m_playback->play();
        }

        m_avTransport.setInstanceVariable(instanceId, AVTransport::Variable::AVTransportURI, uri);
        m_avTransport.setInstanceVariable(instanceId, AVTransport::Variable::AVTransportURIMetaData, metaData);
    }
    catch (std::exception& e)
    {
        log::error(e.what());
        throw AVTransport::IllegalMimeTypeException();
    }
}

void MediaRendererDevice::setNextAVTransportURI(uint32_t instanceId, const std::string& uri, const std::string& metaData)
{
    try
    {
        m_queue.setNextUri(uri);

        m_avTransport.setInstanceVariable(instanceId, AVTransport::Variable::NextAVTransportURI, uri);
        m_avTransport.setInstanceVariable(instanceId, AVTransport::Variable::NextAVTransportURIMetaData, metaData);
    }
    catch (std::exception& e)
    {
        log::error(e.what());
        throw AVTransport::IllegalMimeTypeException();
    }
}

void MediaRendererDevice::stop(uint32_t instanceId)
{
    throwOnBadInstanceId(instanceId);
    m_playback->stop();
}

void MediaRendererDevice::play(uint32_t instanceId, const std::string& speed)
{
    throwOnBadInstanceId(instanceId);

    log::info("Play ({}): speed {}", instanceId, speed);
    m_playback->play();
}

void MediaRendererDevice::seek(uint32_t instanceId, upnp::AVTransport::SeekMode /*mode*/, const std::string& /*target*/)
{
    throwOnBadInstanceId(instanceId);
}

void MediaRendererDevice::next(uint32_t instanceId)
{
    throwOnBadInstanceId(instanceId);

    log::info("Next ({})", instanceId);
    m_playback->next();
}

void MediaRendererDevice::previous(uint32_t instanceId)
{
    throwOnBadInstanceId(instanceId);

    log::info("Previous ({})", instanceId);
    m_playback->prev();
}

void MediaRendererDevice::pause(uint32_t instanceId)
{
    throwOnBadInstanceId(instanceId);

    log::info("Pause ({})", instanceId);
    m_playback->pause();
}

#ifdef HAVE_LIBCEC
void MediaRendererDevice::CheckCecState(PlaybackState state)
{
    if (state == PlaybackState::Playing)
    {
        m_thread.addJob([this] () {
            m_timer.cancel();
            TurnOnCecDevice(m_cecDevice);
        });
    }
    else
    {
        if (!m_timer.isRunning())
        {
            m_timer.run(std::chrono::minutes(g_idleTimeout), [this] () {
                if (m_playback->getState() != PlaybackState::Playing)
                {
                    TurnOffCecDevice(m_cecDevice);
                }
            });
        }
    }
}
#else
void MediaRendererDevice::CheckCecState(PlaybackState) {}
#endif

}
