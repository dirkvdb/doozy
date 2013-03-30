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

#include <sstream>

using namespace utils;
using namespace upnp;
using namespace audio;
using namespace std::placeholders;

namespace doozy
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

MediaRendererDevice::MediaRendererDevice(const std::string& udn, const std::string& descriptionXml, int32_t advertiseIntervalInSeconds)
: m_Playback(PlaybackFactory::create("FFmpeg", "OpenAL", m_Queue))
, m_RootDevice(udn, descriptionXml, advertiseIntervalInSeconds)
, m_ConnectionManager(m_RootDevice, *this)
, m_RenderingControl(m_RootDevice, *this)
, m_AVTransport(m_RootDevice, *this)
{
    m_Playback->PlaybackStateChanged.connect([this] (PlaybackState state) {
        std::string str;
        
        switch (state)
        {
        case PlaybackState::Playing:
            str = AVTransport::toString(AVTransport::State::Playing);
            break;
        case PlaybackState::Paused:
            str = AVTransport::toString(AVTransport::State::PausedPlayback);
            break;
        case PlaybackState::Stopped:
            str = AVTransport::toString(AVTransport::State::Stopped);
            break;
        }
        
        if (!str.empty())
        {
            m_AVTransport.setInstanceVariable(0, AVTransport::Variable::TransportState, str);
        }
    }, this);
    
    m_Playback->AvailableActionsChanged.connect([this] (const std::set<PlaybackAction>& actions) {
        m_AVTransport.setInstanceVariable(0, AVTransport::Variable::CurrentTransportActions, toString(actions));
    }, this);
}

void MediaRendererDevice::start()
{
    m_RootDevice.ControlActionRequested.connect(std::bind(&MediaRendererDevice::onControlActionRequest, this, _1), this);
    m_RootDevice.EventSubscriptionRequested.connect(std::bind(&MediaRendererDevice::onEventSubscriptionRequest, this, _1), this);

    m_RootDevice.initialize();
    setInitialValues();
}

void MediaRendererDevice::stop()
{
    m_RootDevice.ControlActionRequested.disconnect(this);
    m_RootDevice.EventSubscriptionRequested.disconnect(this);

    m_RootDevice.destroy();
}

void MediaRendererDevice::setInitialValues()
{
    m_SupportedProtocols.push_back(ProtocolInfo("http-get:*:audio/mpeg:DLNA.ORG_PN=MP3"));
    m_SupportedProtocols.push_back(ProtocolInfo("http-get:*:audio/flac:*"));
    m_SupportedProtocols.push_back(ProtocolInfo("http-get:*:audio/x-flac:*"));
    m_SupportedProtocols.push_back(ProtocolInfo("http-get:*:audio/L16;rate=44100;channels=1:DLNA.ORG_PN=LPCM"));
    m_SupportedProtocols.push_back(ProtocolInfo("http-get:*:audio/L16;rate=44100;channels=2:DLNA.ORG_PN=LPCM"));
    m_SupportedProtocols.push_back(ProtocolInfo("http-get:*:audio/L16;rate=48000;channels=1:DLNA.ORG_PN=LPCM"));
    m_SupportedProtocols.push_back(ProtocolInfo("http-get:*:audio/L16;rate=48000;channels=2:DLNA.ORG_PN=LPCM"));
    m_SupportedProtocols.push_back(ProtocolInfo("http-get:*:audio/x-ms-wma:DLNA.ORG_PN=WMABASE"));
    m_SupportedProtocols.push_back(ProtocolInfo("http-get:*:audio/x-ms-wma:DLNA.ORG_PN=WMAFULL"));
    m_SupportedProtocols.push_back(ProtocolInfo("http-get:*:audio/mp4:DLNA.ORG_PN=AAC_ISO"));
    m_SupportedProtocols.push_back(ProtocolInfo("http-get:*:audio/3gpp:DLNA.ORG_PN=AAC_ISO"));
    m_SupportedProtocols.push_back(ProtocolInfo("http-get:*:audio/vnd.dlna.adts:DLNA.ORG_PN=AAC_ADTS_320"));
    m_SupportedProtocols.push_back(ProtocolInfo("http-wavetunes:*:audio/x-ms-wma:*"));
    m_SupportedProtocols.push_back(ProtocolInfo("http-get:*:audio/wav:*"));
    m_SupportedProtocols.push_back(ProtocolInfo("http-get:*:audio/x-wav:*"));
    
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
}
    
void MediaRendererDevice::onEventSubscriptionRequest(Upnp_Subscription_Request* pRequest)
{
    log::debug("Renderer: event subscription request %s", pRequest->ServiceId);
    
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
        m_Queue.addTrack(uri);
        
        m_AVTransport.setVariable(AVTransport::Variable::CurrentTrackURI, uri);
        m_AVTransport.setVariable(AVTransport::Variable::CurrentTrackMetaData, metaData);
        
        // setting the current uri, overwrites queued items
        m_AVTransport.setVariable(AVTransport::Variable::NextAVTransportURI, "");
        m_AVTransport.setVariable(AVTransport::Variable::NextAVTransportURIMetaData, "");
    }
    catch (std::exception& e)
    {
        log::error(e.what());
        throw AVTransport::IllegalMimeTypeException();
        
        m_AVTransport.setVariable(AVTransport::Variable::CurrentTrackURI, "");
        m_AVTransport.setVariable(AVTransport::Variable::CurrentTrackMetaData, "");
        m_AVTransport.setVariable(AVTransport::Variable::NextAVTransportURI, "");
        m_AVTransport.setVariable(AVTransport::Variable::NextAVTransportURIMetaData, "");
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
}

void MediaRendererDevice::previous(uint32_t instanceId)
{
    throwOnBadInstanceId(instanceId);
}

void MediaRendererDevice::pause(uint32_t instanceId)
{
    throwOnBadInstanceId(instanceId);
    m_Playback->pause();
}

}
