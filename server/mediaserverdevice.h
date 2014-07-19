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

#ifndef MEDIA_SERVER_DEVICE_H
#define MEDIA_SERVER_DEVICE_H

#include <memory>

#include "utils/workerthread.h"

#include "upnp/upnprootdevice.h"
#include "upnp/upnpdeviceservice.h"
#include "upnp/upnpactionresponse.h"
//#include "upnp/upnpavtransportservice.h"
#include "upnp/upnpcontentdirectoryservice.h"
#include "upnp/upnpconnectionmanagerservice.h"

namespace upnp
{
class WebServer;
}

namespace doozy
{

class IMusicLibrary;

class MediaServerDevice : public upnp::IConnectionManager
                        , public upnp::ContentDirectory::IContentDirectory
                        //, public upnp::IAVTransport
{
public:
    MediaServerDevice(const std::string& udn, const std::string& descriptionXml, int32_t advertiseIntervalInSeconds, upnp::WebServer& webServer, std::unique_ptr<IMusicLibrary> library);
    MediaServerDevice(const MediaServerDevice&) = delete;
    
    void start();
    void stop();
    
    // IConnectionManager
    void prepareForConnection(const upnp::ProtocolInfo& protocolInfo, upnp::ConnectionManager::ConnectionInfo& info) override;
    void connectionComplete(int32_t connectionId) override;
    upnp::ConnectionManager::ConnectionInfo getCurrentConnectionInfo(int32_t connectionId) override;
    
    // IContentDirectory
    std::vector<upnp::Property> GetSearchCapabilities() override;
    std::vector<upnp::Property> GetSortCapabilities() override;
    std::string GetSystemUpdateId() override;
    upnp::ContentDirectory::ActionResult Browse(const std::string& id, upnp::ContentDirectory::BrowseFlag flag, const std::vector<upnp::Property>& filter, uint32_t startIndex, uint32_t count, const std::vector<upnp::ContentDirectory::SortProperty>& sortCriteria) override;
    
    //IAVTransport
//    virtual void setAVTransportURI(uint32_t instanceId, const std::string& uri, const std::string& metaData);
//    virtual void setNextAVTransportURI(uint32_t instanceId, const std::string& uri, const std::string& metaData);
//    virtual void stop(uint32_t instanceId);
//    virtual void play(uint32_t instanceId, const std::string& speed);
//    virtual void seek(uint32_t instanceId, upnp::AVTransport::SeekMode mode, const std::string& target);
//    virtual void next(uint32_t instanceId);
//    virtual void previous(uint32_t instanceId);
//    virtual void pause(uint32_t instanceId);
    
private:
    void setInitialValues();
    //void setTransportVariable(uint32_t instanceId, upnp::AVTransport::Variable var, const std::string& value);
    
    void onEventSubscriptionRequest(Upnp_Subscription_Request* pRequest);
    void onControlActionRequest(Upnp_Action_Request* pRequest);
    
    //void throwOnBadInstanceId(uint32_t id) const;

    mutable std::mutex                          m_Mutex;
    
    upnp::RootDevice                            m_RootDevice;
    upnp::ConnectionManager::Service            m_ConnectionManager;
    upnp::ContentDirectory::Service             m_ContentDirectory;
    //upnp::AVTransport::Service                  m_AVTransport;
    upnp::WebServer&                            m_WebServer;

    std::unique_ptr<IMusicLibrary>              m_Lib;
    
    upnp::ConnectionManager::ConnectionInfo     m_CurrentConnectionInfo;
    utils::WorkerThread                         m_Thread;
};



}

#endif
