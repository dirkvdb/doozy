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

#ifndef MEDIA_RENDERER_DEVICE_H
#define MEDIA_RENDERER_DEVICE_H

#include <memory>

#include "upnp/upnprootdevice.h"
#include "upnp/upnpdeviceservice.h"
#include "upnp/upnpactionresponse.h"
#include "upnp/upnprenderingcontrolservice.h"
#include "upnp/upnpavtransportservice.h"
#include "upnp/upnpconnectionmanagerservice.h"

#include "audio/audioplaybackinterface.h"
#include "playqueue.h"

namespace doozy
{

class MediaRendererDevice : public upnp::IConnectionManager
                          , public upnp::IRenderingControl
                          , public upnp::IAVTransport
{
public:
    MediaRendererDevice(const std::string& udn, const std::string& descriptionXml, int32_t advertiseIntervalInSeconds);
    MediaRendererDevice(const MediaRendererDevice&) = delete;
    
    void start();
    void stop();
    
    // IConnectionManager
    virtual void prepareForConnection(const upnp::ProtocolInfo& protocolInfo, upnp::ConnectionManager::ConnectionInfo& info);
    virtual void connectionComplete(int32_t connectionId);
    virtual upnp::ConnectionManager::ConnectionInfo getCurrentConnectionInfo(int32_t connectionId);
    
    // IRenderingControl
    virtual void selectPreset(uint32_t instanceId, const std::string& name);
    virtual void setVolume(uint32_t instanceId, upnp::RenderingControl::Channel channel, uint16_t value);
    virtual void setMute(uint32_t instanceId, upnp::RenderingControl::Channel channel, bool enabled);
    
    //IAVTransport
    virtual void setAVTransportURI(uint32_t instanceId, const std::string& uri, const std::string& metaData);
    virtual void setNextAVTransportURI(uint32_t instanceId, const std::string& uri, const std::string& metaData);
    virtual void stop(uint32_t instanceId);
    virtual void play(uint32_t instanceId, const std::string& speed);
    virtual void seek(uint32_t instanceId, upnp::AVTransport::SeekMode mode, const std::string& target);
    virtual void next(uint32_t instanceId);
    virtual void previous(uint32_t instanceId);
    virtual void pause(uint32_t instanceId);
    
private:
    void setInitialValues();
    void onEventSubscriptionRequest(Upnp_Subscription_Request* pRequest);
    void onControlActionRequest(Upnp_Action_Request* pRequest);
    bool supportsProtocol(const upnp::ProtocolInfo& info) const;
    
    void throwOnBadInstanceId(uint32_t id) const;

    mutable std::mutex                          m_Mutex;
    
    PlayQueue                                   m_Queue;
    std::unique_ptr<audio::IPlayback>           m_Playback;
    
    upnp::RootDevice                            m_RootDevice;
    upnp::ConnectionManager::Service            m_ConnectionManager;
    upnp::RenderingControl::Service             m_RenderingControl;
    upnp::AVTransport::Service                  m_AVTransport;
    
    std::vector<upnp::ProtocolInfo>             m_SupportedProtocols;
    upnp::ConnectionManager::ConnectionInfo     m_CurrentConnectionInfo;
};



}

#endif
