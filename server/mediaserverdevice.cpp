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

#include "mediaserverdevice.h"

#include "utils/log.h"
#include "utils/stringoperations.h"
#include "utils/fileoperations.h"

#include "upnp/upnpwebserver.h"

#include "library/musiclibraryinterface.h"

#include <sstream>

using namespace utils;
using namespace upnp;
using namespace std::placeholders;


namespace doozy
{

MediaServerDevice::MediaServerDevice(const std::string& udn, const std::string& descriptionXml, int32_t advertiseIntervalInSeconds, upnp::WebServer& webServer, std::unique_ptr<IMusicLibrary> library)
: m_RootDevice(udn, descriptionXml, advertiseIntervalInSeconds)
, m_ConnectionManager(m_RootDevice, *this)
, m_ContentDirectory(m_RootDevice, *this)
//, m_AVTransport(m_RootDevice, *this)
, m_WebServer(webServer)
, m_Lib(std::move(library))
{
}

void MediaServerDevice::start()
{
    m_Thread.start();

    m_RootDevice.ControlActionRequested.connect(std::bind(&MediaServerDevice::onControlActionRequest, this, _1), this);
    m_RootDevice.EventSubscriptionRequested.connect(std::bind(&MediaServerDevice::onEventSubscriptionRequest, this, _1), this);

    m_RootDevice.initialize();
    setInitialValues();

    m_Lib->scan(true);
}

void MediaServerDevice::stop()
{
    m_Thread.stop();

    m_RootDevice.ControlActionRequested.disconnect(this);
    m_RootDevice.EventSubscriptionRequested.disconnect(this);

    m_RootDevice.destroy();
}

void MediaServerDevice::setInitialValues()
{
    m_ConnectionManager.setVariable(ConnectionManager::Variable::SourceProtocolInfo, "");
    m_ConnectionManager.setVariable(ConnectionManager::Variable::SinkProtocolInfo, "");
    m_ConnectionManager.setVariable(ConnectionManager::Variable::CurrentConnectionIds, "0");
    m_ConnectionManager.setVariable(ConnectionManager::Variable::ArgumentTypeConnectionStatus, "OK");
    
    m_CurrentConnectionInfo.connectionStatus    = ConnectionManager::ConnectionStatus::Ok;
    m_CurrentConnectionInfo.direction           = ConnectionManager::Direction::Output;
}

//void MediaServerDevice::setTransportVariable(uint32_t instanceId, AVTransport::Variable var, const std::string& value)
//{
//    // Set the variable on the workerthread to avoid blocking the playback thread
//    m_Thread.addJob([=] () {
//        m_AVTransport.setInstanceVariable(instanceId, var, value);
//    });
//}

void MediaServerDevice::onEventSubscriptionRequest(Upnp_Subscription_Request* pRequest)
{
    //log::debug("Renderer: event subscription request %s", pRequest->ServiceId);
    
    switch (serviceIdUrnStringToService(pRequest->ServiceId))
    {
    //case ServiceType::AVTransport:              return m_RootDevice.acceptSubscription(pRequest->ServiceId, pRequest->Sid, m_AVTransport.getSubscriptionResponse());
    case ServiceType::ContentDirectory:         return m_RootDevice.acceptSubscription(pRequest->ServiceId, pRequest->Sid, m_ContentDirectory.getSubscriptionResponse());
    case ServiceType::ConnectionManager:        return m_RootDevice.acceptSubscription(pRequest->ServiceId, pRequest->Sid, m_ConnectionManager.getSubscriptionResponse());
    default:
        log::warn("Invalid event subscription request: %s", pRequest->ServiceId);
    }
}

void MediaServerDevice::onControlActionRequest(Upnp_Action_Request* pRequest)
{
    //log::debug("Renderer: action request: %s", pRequest->ActionName);
    
    xml::Document requestDoc(pRequest->ActionRequest, xml::Document::NoOwnership);
    //log::debug(requestDoc.toString());
    
    switch (serviceIdUrnStringToService(pRequest->ServiceID))
    {
//    case ServiceType::AVTransport:
//        pRequest->ActionResult = m_AVTransport.onAction(pRequest->ActionName, requestDoc).getActionDocument();
//        break;
    case ServiceType::ContentDirectory:
        pRequest->ActionResult = m_ContentDirectory.onAction(pRequest->ActionName, requestDoc).getActionDocument();
        break;
    case ServiceType::ConnectionManager:
        pRequest->ActionResult = m_ConnectionManager.onAction(pRequest->ActionName, requestDoc).getActionDocument();
        break;
    default:
        throw ServiceException("Invalid subscribtionId", 401);
    }


    xml::Document responesDoc(pRequest->ActionResult, xml::Document::NoOwnership);
    log::debug(responesDoc.toString());
}

//void MediaServerDevice::throwOnBadInstanceId(uint32_t id) const
//{
//    if (id != 0)
//    {
//        throw AVTransport::InvalidInstanceIdException();
//    }
//}

/***************************************************
 * Connection Manager calls
 ***************************************************/

void MediaServerDevice::prepareForConnection(const ProtocolInfo& protocolInfo, ConnectionManager::ConnectionInfo& info)
{
    if (info.direction != ConnectionManager::Direction::Input)
    {
        throw ConnectionManager::IncompatibleDirectionsException();
    }
    
    // currently we only support one instance
    info.connectionId = 0;
    info.avTransportId = 0;
    info.renderingControlServiceId = 0;
}

void MediaServerDevice::connectionComplete(int32_t connectionId)
{
    // No actions necessary because instances are not supported yet
}

upnp::ConnectionManager::ConnectionInfo MediaServerDevice::getCurrentConnectionInfo(int32_t connectionId)
{
    if (connectionId != 0)
    {
        throw ConnectionManager::InvalidConnectionReferenceException();
    }

    return m_CurrentConnectionInfo;
}

/***************************************************
 * ContentDirectory calls
 ***************************************************/

std::vector<upnp::Property> MediaServerDevice::GetSearchCapabilities()
{
    return {};
}

std::vector<Property> MediaServerDevice::GetSortCapabilities()
{
    return {
        Property::Title
    };
}

std::string MediaServerDevice::GetSystemUpdateId()
{
    return "0";
}

ContentDirectory::ActionResult MediaServerDevice::Browse(const std::string& id, ContentDirectory::BrowseFlag flag, const std::vector<Property>& filter, uint32_t startIndex, uint32_t count, const std::vector<ContentDirectory::SortProperty>& sortCriteria)
{
    ContentDirectory::ActionResult result;

    log::debug("Browse: %s (%d - %d)", id, startIndex, count);

    result.result = m_Lib->getItems(id, startIndex, count);
    result.updateId = 1;
    result.totalMatches = m_Lib->getObjectCountInContainer(id);
    result.numberReturned = static_cast<uint32_t>(result.result.size());

    log::debug("Browse result: Update id %d (ret %d - #matches %d)", result.updateId, result.numberReturned, result.totalMatches);
    for (auto& item : result.result)
    {
        log::debug(item->getTitle());
        if (item->isContainer())
        {
            item->setChildCount(m_Lib->getObjectCountInContainer(item->getObjectId()));
        }
    }
    
    return result;
}

/***************************************************
 * AVTransport calls
 ***************************************************/

//void MediaServerDevice::setAVTransportURI(uint32_t instanceId, const std::string& uri, const std::string& metaData)
//{
//    throwOnBadInstanceId(instanceId);
//    
//    try
//    {
//        log::info("Play uri (%d): %s", instanceId, uri);
//        m_Queue.setCurrentUri(uri);
//        if (m_Playback->isPlaying())
//        {
//            m_Playback->stop();
//            m_Playback->play();
//        }
//        
//        m_AVTransport.setInstanceVariable(instanceId, AVTransport::Variable::AVTransportURI, uri);
//        m_AVTransport.setInstanceVariable(instanceId, AVTransport::Variable::AVTransportURIMetaData, metaData);
//    }
//    catch (std::exception& e)
//    {
//        log::error(e.what());
//        throw AVTransport::IllegalMimeTypeException();
//    }
//}
//
//void MediaServerDevice::setNextAVTransportURI(uint32_t instanceId, const std::string& uri, const std::string& metaData)
//{
//    try
//    {
//        m_Queue.setNextUri(uri);
//        
//        m_AVTransport.setInstanceVariable(instanceId, AVTransport::Variable::NextAVTransportURI, uri);
//        m_AVTransport.setInstanceVariable(instanceId, AVTransport::Variable::NextAVTransportURIMetaData, metaData);
//    }
//    catch (std::exception& e)
//    {
//        log::error(e.what());
//        throw AVTransport::IllegalMimeTypeException();
//    }
//}
//
//void MediaServerDevice::stop(uint32_t instanceId)
//{
//    throwOnBadInstanceId(instanceId);
//    m_Playback->stop();
//}
//
//void MediaServerDevice::play(uint32_t instanceId, const std::string& speed)
//{
//    throwOnBadInstanceId(instanceId);
//    
//    log::info("Play (%d): speed %s", instanceId, speed);
//    m_Playback->play();
//}
//
//void MediaServerDevice::seek(uint32_t instanceId, upnp::AVTransport::SeekMode mode, const std::string& target)
//{
//    throwOnBadInstanceId(instanceId);
//}
//
//void MediaServerDevice::next(uint32_t instanceId)
//{
//    throwOnBadInstanceId(instanceId);
//    m_Playback->next();
//}
//
//void MediaServerDevice::previous(uint32_t instanceId)
//{
//    throwOnBadInstanceId(instanceId);
//    m_Playback->prev();
//}
//
//void MediaServerDevice::pause(uint32_t instanceId)
//{
//    throwOnBadInstanceId(instanceId);
//    m_Playback->pause();
//}

}
