//    Copyright (C) 2013 Dirk Vanden Boer <dirk.vdb@gmail.com>
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

#include "doozycontrolpoint.h"

#include "utils/log.h"

using namespace utils;
using namespace utils::stringops;

namespace doozy
{

ControlPoint::ControlPoint()
: m_Cp(m_Client)
, m_MediaServer(m_Client)
, m_RendererScanner(m_Client, upnp::Device::Type::MediaRenderer)
, m_ServerScanner(m_Client, upnp::Device::Type::MediaServer)
{
    run();
}

void ControlPoint::run()
{
    try
    {
        m_Client.initialize();
        m_ServerScanner.start();
        m_ServerScanner.refresh();
        m_RendererScanner.start();
        m_RendererScanner.refresh();
        m_Cp.activate();
        m_Webserver.reset(new upnp::WebServer("/Users/dirk/Projects/doozy/controlpoint"));
        
        log::info("Webserver listening url: %s", m_Webserver->getWebRootUrl());
        
    }
    catch(std::exception& e)
    {
        log::error(e.what());
    }
}
    
void ControlPoint::stop()
{
    m_Webserver.reset();
    m_RendererScanner.stop();
    m_ServerScanner.stop();
    m_Cp.deactivate();
    m_Client.destroy();
}
    
void ControlPoint::GetRenderers(rpc::DeviceResponse& response)
{
    log::info("Get renderers");
    
    auto devs = m_RendererScanner.getDevices();
    std::vector<rpc::Device> rpcDevs;
    std::transform(devs.begin(), devs.end(), std::back_inserter(rpcDevs), [&] (const std::pair<std::string, const std::shared_ptr<upnp::Device>>& dev) {
        rpc::Device rpcDev;
        rpcDev.__set_name(dev.second->m_FriendlyName);
        rpcDev.__set_udn(dev.second->m_UDN);
        return rpcDev;
    });

    response.__set_devices(rpcDevs);
}
    
void ControlPoint::GetServers(rpc::DeviceResponse& response)
{
    log::info("Get servers");
    
    auto devs = m_ServerScanner.getDevices();
    std::vector<rpc::Device> rpcDevs;
    std::transform(devs.begin(), devs.end(), std::back_inserter(rpcDevs), [&] (const std::pair<std::string, const std::shared_ptr<upnp::Device>>& dev) {
        rpc::Device rpcDev;
        rpcDev.__set_name(dev.second->m_FriendlyName);
        rpcDev.__set_udn(dev.second->m_UDN);
        return rpcDev;
    });
    
    response.__set_devices(rpcDevs);
    log::info("Get servers returned %d servers", devs.size());
}

static rpc::ItemClass::type convertClass(upnp::Item::Class c)
{
    switch (c)
    {
        case upnp::Item::Class::Container:
        case upnp::Item::Class::AudioContainer:
        case upnp::Item::Class::VideoContainer:
        case upnp::Item::Class::ImageContainer:
            return rpc::ItemClass::Container;
        case upnp::Item::Class::Video:
            return rpc::ItemClass::VideoItem;
        case upnp::Item::Class::Audio:
            return rpc::ItemClass::AudioItem;
        case upnp::Item::Class::Image:
            return rpc::ItemClass::ImageItem;
        case upnp::Item::Class::Generic:
            return rpc::ItemClass::Item;
        default:
            return rpc::ItemClass::Unknown;
    }
}

void ControlPoint::Browse(rpc::BrowseResponse& response, const rpc::BrowseRequest& request)
{
    auto item = std::make_shared<upnp::Item>(request.containerid);
    m_MediaServer.setDevice(m_ServerScanner.getDevice(request.udn));
    auto items = m_MediaServer.getAllInContainer(item);
    
    for (auto& item : items)
    {
        rpc::Item i;
        i.id = item->getObjectId();
        i.title = item->getTitle();
        i.itemclass = convertClass(item->getClass());
        
        std::string url;
        if (item->getClass() == upnp::Item::Class::AudioContainer)
        {
            url = item->getMetaData(upnp::Property::AlbumArt);
        }
        
        if (url.empty())
        {
            url = item->getAlbumArtUri(upnp::dlna::ProfileId::JpegThumbnail);
        }

        if (!url.empty())
        {
            i.__set_thumbnailurl(url);
        }

        response.items.push_back(i);
    }
}
    
}