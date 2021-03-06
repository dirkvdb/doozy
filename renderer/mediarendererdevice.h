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

#pragma once

#include "utils/timer.h"
#include "utils/workerthread.h"

#include "upnp/asio.h"
#include "upnp/upnp.rootdevice.h"
#include "upnp/upnp.actionresponse.h"
#include "upnp/upnp.renderingcontrol.service.h"
#include "upnp/upnp.avtransport.service.h"
#include "upnp/upnp.connectionmanager.service.h"

#include "renderersettings.h"
#include "common/doozydeviceinterface.h"

#include "audio/audioplaybackinterface.h"
#include "playqueue.h"
#include "doozyconfig.h"


namespace upnp
{
class WebServer;
}

namespace doozy
{

class CecControl;

class MediaRendererDevice : public upnp::IConnectionManager
                          , public upnp::IRenderingControl
                          , public upnp::IAVTransport
                          , public IDevice
{
public:
    MediaRendererDevice(RendererSettings& settings);
    MediaRendererDevice(const MediaRendererDevice&) = delete;

    ~MediaRendererDevice();

    void start(const std::string& networkInterface) override;
    void stop() override;

    // IConnectionManager
    void prepareForConnection(const upnp::ProtocolInfo& protocolInfo, upnp::ConnectionManager::ConnectionInfo& info) override;
    void connectionComplete(int32_t connectionId) override;
    upnp::ConnectionManager::ConnectionInfo getCurrentConnectionInfo(int32_t connectionId) override;

    // IRenderingControl
    void selectPreset(uint32_t instanceId, const std::string& name) override;
    void setVolume(uint32_t instanceId, upnp::RenderingControl::Channel channel, uint16_t value) override;
    void setMute(uint32_t instanceId, upnp::RenderingControl::Channel channel, bool enabled) override;

    //IAVTransport
    void setAVTransportURI(uint32_t instanceId, const std::string& uri, const std::string& metaData) override;
    void setNextAVTransportURI(uint32_t instanceId, const std::string& uri, const std::string& metaData) override;
    void stop(uint32_t instanceId) override;
    void play(uint32_t instanceId, const std::string& speed) override;
    void seek(uint32_t instanceId, upnp::AVTransport::SeekMode mode, const std::string& target) override;
    void next(uint32_t instanceId) override;
    void previous(uint32_t instanceId) override;
    void pause(uint32_t instanceId) override;

private:
    void setInitialValues();
    void setTransportVariable(uint32_t instanceId, upnp::AVTransport::Variable var, const std::string& value);

    upnp::DeviceSubscriptionResponse onEventSubscriptionRequest(const upnp::SubscriptionRequest& request);
    std::string onControlActionRequest(const upnp::ActionRequest& request);
    bool supportsProtocol(const upnp::ProtocolInfo& info) const;
    void addAlbumArtToWebServer(const PlayQueueItemPtr& item);

    void throwOnBadInstanceId(uint32_t id) const;
    void CheckCecState(audio::PlaybackState state);
    void StartCecTimer();
    void AbortCecTimer();

    void signalHandler(const boost::system::error_code& error, int signal_number);

    asio::io_service                            m_io;
    RendererSettings                            m_settings;
    bool                                        m_stop;

    PlayQueue                                   m_queue;
    std::unique_ptr<audio::IPlayback>           m_playback;

    upnp::RootDevice                            m_rootDevice;
    upnp::ConnectionManager::Service            m_connectionManager;
    upnp::RenderingControl::Service             m_renderingControl;
    upnp::AVTransport::Service                  m_avTransport;

    std::vector<upnp::ProtocolInfo>             m_supportedProtocols;
    upnp::ConnectionManager::ConnectionInfo     m_currentConnectionInfo;

    utils::WorkerThread                         m_thread;
    std::string                                 m_cecDevice;

#ifdef HAVE_LIBCEC
    utils::Timer                                m_timer;
#endif
};

}
