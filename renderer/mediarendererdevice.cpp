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

#include "mediarendererdevice.h"

#include "utils/log.h"
#include "utils/stringoperations.h"
#include "utils/fileoperations.h"

#include "audio/audioplaybackfactory.h"
#include "upnp/upnpwebserver.h"
#include "upnp/upnputils.h"

#include "typeconversions.h"

#include <sstream>

using namespace utils;
using namespace upnp;
using namespace audio;
using namespace std::placeholders;


namespace doozy
{

static std::string toString(const std::set<PlaybackAction>& actions)
{
    std::stringstream ss;
    for (auto& action : actions)
    {
        if (ss.tellp() > 0) ss << ',';
        
        ss << action;
    }
    
    return ss.str();
}

static AVTransport::State PlaybackStateToTransportState(PlaybackState state)
{
    switch (state)
    {
    case PlaybackState::Playing:        return AVTransport::State::Playing;
    case PlaybackState::Paused:         return AVTransport::State::PausedPlayback;
    case PlaybackState::Stopped:
    default:                            return AVTransport::State::Stopped;
    }
}

MediaRendererDevice::MediaRendererDevice(const std::string& udn, const std::string& descriptionXml, int32_t advertiseIntervalInSeconds,
                                         const std::string& audioOutput, const std::string& audioDevice, upnp::WebServer& webServer)
: m_Playback(PlaybackFactory::create("Custom", "Doozy", audioOutput, audioDevice, m_Queue))
, m_RootDevice(udn, descriptionXml, advertiseIntervalInSeconds)
, m_ConnectionManager(m_RootDevice, *this)
, m_RenderingControl(m_RootDevice, *this)
, m_AVTransport(m_RootDevice, *this)
, m_WebServer(webServer)
{
    m_Playback->PlaybackStateChanged.connect([this] (auto state) {
        setTransportVariable(0, AVTransport::Variable::TransportState, AVTransport::toString(PlaybackStateToTransportState(state)));
    }, this);
    
    m_Playback->AvailableActionsChanged.connect([this] (auto actions) {
        setTransportVariable(0, AVTransport::Variable::CurrentTransportActions, toString(actions));
    }, this);
    
    m_Playback->ProgressChanged.connect([this] (auto progress) {
        setTransportVariable(0, AVTransport::Variable::RelativeTimePosition, durationToString(progress));
    }, this);
    
    m_Playback->NewTrackStarted.connect([this] (auto track) {
        auto item = std::dynamic_pointer_cast<PlayQueueItem>(track);
        assert(item);
        
        addAlbumArtToWebServer(item);
    
        setTransportVariable(0, AVTransport::Variable::CurrentTrackURI,         item->getUri());
        setTransportVariable(0, AVTransport::Variable::CurrentTrackMetaData,    item->getMetadataString());
        setTransportVariable(0, AVTransport::Variable::AVTransportURI,          item->getAVTransportUri());
        setTransportVariable(0, AVTransport::Variable::NextAVTransportURI,      m_Queue.getNextUri());
        setTransportVariable(0, AVTransport::Variable::CurrentTrackDuration,    durationToString(m_Playback->getDuration()));
        setTransportVariable(0, AVTransport::Variable::NumberOfTracks,          std::to_string(m_Queue.getNumberOfTracks()));
    }, this);
}

void MediaRendererDevice::start()
{
    m_Thread.start();

    m_RootDevice.ControlActionRequested.connect(std::bind(&MediaRendererDevice::onControlActionRequest, this, _1), this);
    m_RootDevice.EventSubscriptionRequested.connect(std::bind(&MediaRendererDevice::onEventSubscriptionRequest, this, _1), this);

    m_RootDevice.initialize();
    setInitialValues();
}

void MediaRendererDevice::stop()
{
    m_Thread.stop();

    m_RootDevice.ControlActionRequested.disconnect(this);
    m_RootDevice.EventSubscriptionRequested.disconnect(this);

    m_RootDevice.destroy();
}

void MediaRendererDevice::setInitialValues()
{
#if defined(HAVE_MAD) || defined(HAVE_FFMPEG)
    m_SupportedProtocols.push_back(ProtocolInfo("http-get:*:audio/mpeg:DLNA.ORG_PN=MP3"));
#endif

#if defined(HAVE_FLAC) || defined(HAVE_FFMPEG)
    m_SupportedProtocols.push_back(ProtocolInfo("http-get:*:audio/flac:*"));
    m_SupportedProtocols.push_back(ProtocolInfo("http-get:*:audio/x-flac:*"));
#endif

#ifdef HAVE_FFMPEG // assume ffmpeg supports these formats (possibly make this more smart and actually check ffmpeg config options)
    m_SupportedProtocols.push_back(ProtocolInfo("http-get:*:audio/L16;rate=44100;channels=1:DLNA.ORG_PN=LPCM"));
    m_SupportedProtocols.push_back(ProtocolInfo("http-get:*:audio/L16;rate=44100;channels=2:DLNA.ORG_PN=LPCM"));
    //m_SupportedProtocols.push_back(ProtocolInfo("http-get:*:audio/L16;rate=48000;channels=1:DLNA.ORG_PN=LPCM"));
    //m_SupportedProtocols.push_back(ProtocolInfo("http-get:*:audio/L16;rate=48000;channels=2:DLNA.ORG_PN=LPCM"));
    m_SupportedProtocols.push_back(ProtocolInfo("http-get:*:audio/x-ms-wma:DLNA.ORG_PN=WMABASE"));
    m_SupportedProtocols.push_back(ProtocolInfo("http-get:*:audio/x-ms-wma:DLNA.ORG_PN=WMAFULL"));
    m_SupportedProtocols.push_back(ProtocolInfo("http-get:*:audio/mp4:DLNA.ORG_PN=AAC_ISO"));
    m_SupportedProtocols.push_back(ProtocolInfo("http-get:*:audio/3gpp:DLNA.ORG_PN=AAC_ISO"));
    //m_SupportedProtocols.push_back(ProtocolInfo("http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=AAC_ADTS_320"));
    m_SupportedProtocols.push_back(ProtocolInfo("http-wavetunes:*:audio/x-ms-wma:*"));
    m_SupportedProtocols.push_back(ProtocolInfo("http-get:*:audio/wav:*"));
    m_SupportedProtocols.push_back(ProtocolInfo("http-get:*:audio/x-wav:*"));
#endif
    
    std::stringstream ss;
    for (auto& protocol : m_SupportedProtocols)
    {
        if (ss.tellp() > 0)
        {
            ss << ',';
        }
    
        ss << protocol.toString();
    }
    
    m_ConnectionManager.setVariable(ConnectionManager::Variable::SourceProtocolInfo, "");
    m_ConnectionManager.setVariable(ConnectionManager::Variable::SinkProtocolInfo, ss.str());
    m_ConnectionManager.setVariable(ConnectionManager::Variable::CurrentConnectionIds, "0");
    m_ConnectionManager.setVariable(ConnectionManager::Variable::ArgumentTypeConnectionStatus, "OK");
    
    m_CurrentConnectionInfo.connectionStatus    = ConnectionManager::ConnectionStatus::Ok;
    m_CurrentConnectionInfo.direction           = ConnectionManager::Direction::Input;
    
    m_RenderingControl.setVariable(RenderingControl::Variable::PresetNameList, "FactoryDefaults");
    m_RenderingControl.setVolume(0, RenderingControl::Channel::Master, m_Playback->getVolume());
    m_RenderingControl.setMute(0, RenderingControl::Channel::Master, m_Playback->getMute());
    
    m_AVTransport.setInstanceVariable(0, AVTransport::Variable::CurrentTransportActions, toString(m_Playback->getAvailableActions()));
    m_AVTransport.setInstanceVariable(0, AVTransport::Variable::PlaybackStorageMedium, "NETWORK");
    m_AVTransport.setInstanceVariable(0, AVTransport::Variable::TransportState, AVTransport::toString(PlaybackStateToTransportState(m_Playback->getState())));
    m_AVTransport.setInstanceVariable(0, AVTransport::Variable::CurrentPlayMode, toString(AVTransport::PlayMode::Normal));
    m_AVTransport.setInstanceVariable(0, AVTransport::Variable::NumberOfTracks, std::to_string(m_Queue.getNumberOfTracks()));
    m_AVTransport.setInstanceVariable(0, AVTransport::Variable::CurrentTrackDuration, durationToString(0));
    m_AVTransport.setInstanceVariable(0, AVTransport::Variable::RelativeTimePosition, durationToString(0));
    m_AVTransport.setInstanceVariable(0, AVTransport::Variable::AbsoluteTimePosition, "NOT_IMPLEMENTED");
}

void MediaRendererDevice::setTransportVariable(uint32_t instanceId, AVTransport::Variable var, const std::string& value)
{
    // Set the variable on the workerthread to avoid blocking the playback thread
    m_Thread.addJob([=] () {
        m_AVTransport.setInstanceVariable(instanceId, var, value);
    });
}

void MediaRendererDevice::onEventSubscriptionRequest(Upnp_Subscription_Request* pRequest)
{
    //log::debug("Renderer: event subscription request %s", pRequest->ServiceId);
    
    switch (serviceIdUrnStringToService(pRequest->ServiceId))
    {
    case ServiceType::AVTransport:              return m_RootDevice.acceptSubscription(pRequest->ServiceId, pRequest->Sid, m_AVTransport.getSubscriptionResponse());
    case ServiceType::RenderingControl:         return m_RootDevice.acceptSubscription(pRequest->ServiceId, pRequest->Sid, m_RenderingControl.getSubscriptionResponse());
    case ServiceType::ConnectionManager:        return m_RootDevice.acceptSubscription(pRequest->ServiceId, pRequest->Sid, m_ConnectionManager.getSubscriptionResponse());
    default:
        log::warn("Invalid event subscription request: %s", pRequest->ServiceId);
    }
}

void MediaRendererDevice::onControlActionRequest(Upnp_Action_Request* pRequest)
{
    //log::debug("Renderer: action request: %s", pRequest->ActionName);
    
    xml::Document requestDoc(pRequest->ActionRequest, xml::Document::NoOwnership);
    //log::debug(requestDoc.toString());
    
    switch (serviceIdUrnStringToService(pRequest->ServiceID))
    {
    case ServiceType::AVTransport:
        pRequest->ActionResult = m_AVTransport.onAction(pRequest->ActionName, requestDoc).getActionDocument();
        break;
    case ServiceType::RenderingControl:
        pRequest->ActionResult = m_RenderingControl.onAction(pRequest->ActionName, requestDoc).getActionDocument();
        break;
    case ServiceType::ConnectionManager:
        pRequest->ActionResult = m_ConnectionManager.onAction(pRequest->ActionName, requestDoc).getActionDocument();
        break;
    default:
        throw ServiceException("Invalid subscribtionId", 401);
    }
}

bool MediaRendererDevice::supportsProtocol(const ProtocolInfo& info) const
{
    for (auto& protocol : m_SupportedProtocols)
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
        m_WebServer.removeFile("Doozy", "albumartthumb.jpg");
        m_WebServer.addFile("Doozy", "albumartthumb.jpg", "image/jpeg", thumb);
        item->setAlbumArtUri(m_WebServer.getWebRootUrl() + "Doozy/albumartthumb.jpg", upnp::dlna::ProfileId::JpegThumbnail);
    }
    
    auto art = item->getAlbumArt();
    if (!art.empty())
    {
        m_WebServer.removeFile("Doozy", "albumart.jpg");
        m_WebServer.addFile("Doozy", "albumart.jpg", "image/jpeg", art);
        item->setAlbumArtUri(m_WebServer.getWebRootUrl() + "Doozy/albumart.jpg", upnp::dlna::ProfileId::JpegLarge);
    }
}

void MediaRendererDevice::throwOnBadInstanceId(uint32_t id) const
{
    if (id != 0)
    {
        throw AVTransport::InvalidInstanceIdException();
    }
}

/***************************************************
 * Rendering control calls
 ***************************************************/

void MediaRendererDevice::selectPreset(uint32_t instanceId, const std::string& name)
{
    throwOnBadInstanceId(instanceId);
}

void MediaRendererDevice::setVolume(uint32_t instanceId, RenderingControl::Channel channel, uint16_t value)
{
    throwOnBadInstanceId(instanceId);

    if (value > 100 || channel != RenderingControl::Channel::Master)
    {
        throw InvalidArgumentsServiceException();
    }
    
    m_Playback->setVolume(value);
    m_RenderingControl.setVolume(instanceId, channel, m_Playback->getVolume());
}

void MediaRendererDevice::setMute(uint32_t instanceId, RenderingControl::Channel channel, bool enabled)
{
    m_Playback->setMute(enabled);
    m_RenderingControl.setMute(instanceId, channel, m_Playback->getMute());
}


/***************************************************
 * Connection Manager calls
 ***************************************************/

void MediaRendererDevice::prepareForConnection(const ProtocolInfo& protocolInfo, ConnectionManager::ConnectionInfo& info)
{
    if (info.direction != ConnectionManager::Direction::Input)
    {
        throw ConnectionManager::IncompatibleDirectionsException();
    }
    
    if (!supportsProtocol(protocolInfo))
    {
        throw ConnectionManager::IncompatibleProtocolException();
    }
    
    // currently we only support one instance
    info.connectionId = 0;
    info.avTransportId = 0;
    info.renderingControlServiceId = 0;
}

void MediaRendererDevice::connectionComplete(int32_t connectionId)
{
    // No actions necessary because instances are not supported yet
}

upnp::ConnectionManager::ConnectionInfo MediaRendererDevice::getCurrentConnectionInfo(int32_t connectionId)
{
    if (connectionId != 0)
    {
        throw ConnectionManager::InvalidConnectionReferenceException();
    }

    return m_CurrentConnectionInfo;
}

/***************************************************
 * AVTransport calls
 ***************************************************/

void MediaRendererDevice::setAVTransportURI(uint32_t instanceId, const std::string& uri, const std::string& metaData)
{
    throwOnBadInstanceId(instanceId);
    
    try
    {
        log::info("Play uri (%d): %s", instanceId, uri);
        m_Queue.setCurrentUri(uri);
        if (m_Playback->isPlaying())
        {
            m_Playback->stop();
            m_Playback->play();
        }
        
        m_AVTransport.setInstanceVariable(instanceId, AVTransport::Variable::AVTransportURI, uri);
        m_AVTransport.setInstanceVariable(instanceId, AVTransport::Variable::AVTransportURIMetaData, metaData);
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
        m_Queue.setNextUri(uri);
        
        m_AVTransport.setInstanceVariable(instanceId, AVTransport::Variable::NextAVTransportURI, uri);
        m_AVTransport.setInstanceVariable(instanceId, AVTransport::Variable::NextAVTransportURIMetaData, metaData);
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
    m_Playback->stop();
}

void MediaRendererDevice::play(uint32_t instanceId, const std::string& speed)
{
    throwOnBadInstanceId(instanceId);
    
    log::info("Play (%d): speed %s", instanceId, speed);
    m_Playback->play();
}

void MediaRendererDevice::seek(uint32_t instanceId, upnp::AVTransport::SeekMode mode, const std::string& target)
{
    throwOnBadInstanceId(instanceId);
}

void MediaRendererDevice::next(uint32_t instanceId)
{
    throwOnBadInstanceId(instanceId);
    m_Playback->next();
}

void MediaRendererDevice::previous(uint32_t instanceId)
{
    throwOnBadInstanceId(instanceId);
    m_Playback->prev();
}

void MediaRendererDevice::pause(uint32_t instanceId)
{
    throwOnBadInstanceId(instanceId);
    m_Playback->pause();
}

}
