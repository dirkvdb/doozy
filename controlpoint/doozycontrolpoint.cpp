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
#include "publishchannel.h"

using namespace utils;
using namespace utils::stringops;

namespace doozy
{

using namespace google::protobuf;

ControlPoint::ControlPoint(PublishChannel& pubChannel)
: m_Cp(m_Client)
, m_MediaServer(m_Client)
, m_RendererScanner(m_Client, upnp::DeviceType::MediaRenderer)
, m_ServerScanner(m_Client, upnp::DeviceType::MediaServer)
, m_events(&pubChannel)
{
    run();
}

void ControlPoint::run()
{
    try
    {
        m_RendererScanner.DeviceDiscoveredEvent.connect([&] (auto dev) {
            log::info("Device discovered: %s", dev->m_FriendlyName);
            proto::Device protoDev;
            protoDev.set_udn(dev->m_UDN);
            protoDev.set_name(dev->m_FriendlyName);
            m_events.DeviceDiscovered(nullptr, &protoDev, nullptr, nullptr);
        }, this);
    
        m_Client.initialize();
        m_ServerScanner.start();
        m_ServerScanner.refresh();
        m_RendererScanner.start();
        m_RendererScanner.refresh();
        m_Webserver = std::make_unique<upnp::WebServer>("/Users/dirk/Projects/doozy/controlpoint");

        m_Cp.setWebserver(*m_Webserver);
        m_Cp.activate();
        
        log::info("Webserver listening url: %s", m_Webserver->getWebRootUrl());
        
    }
    catch(std::exception& e)
    {
        log::error(e.what());
    }
}
    
void ControlPoint::stop()
{
    m_Cp.deactivate();
    m_Webserver.reset();
    m_RendererScanner.stop();
    m_ServerScanner.stop();
    m_Client.destroy();
}

static proto::ItemClass convertClass(upnp::Class c)
{
    switch (c)
    {
        case upnp::Class::Container:        return proto::ItemClassContainer;
        case upnp::Class::AudioContainer:   return proto::ItemClassAudioContainer;
        case upnp::Class::VideoContainer:   return proto::ItemClassVideoContainer;
        case upnp::Class::ImageContainer:   return proto::ItemClassImageContainer;
        case upnp::Class::Video:            return proto::ItemClassVideoItem;
        case upnp::Class::Audio:            return proto::ItemClassAudioItem;
        case upnp::Class::Image:            return proto::ItemClassImageItem;
        case upnp::Class::Generic:          return proto::ItemClassItem;
        default:
            throw std::runtime_error("Unknown class provided");
    }
}

static proto::Action convertAction(upnp::MediaRenderer::Action action)
{
    switch (action)
    {
        case upnp::MediaRenderer::Action::Play:     return proto::ActionPlay;
        case upnp::MediaRenderer::Action::Next:     return proto::ActionNext;
        case upnp::MediaRenderer::Action::Previous: return proto::ActionPrevious;
        case upnp::MediaRenderer::Action::Seek:     return proto::ActionSeek;
        case upnp::MediaRenderer::Action::Stop:     return proto::ActionStop;
        case upnp::MediaRenderer::Action::Pause:    return proto::ActionPause;
        default:
            throw std::runtime_error("Unknown action provided");
    }
}

void ControlPoint::GetRenderers(RpcController* controller, const proto::Void* request, proto::DeviceResponse* response, Closure* done)
{
    log::info("Get renderers");
    
    auto devs = m_RendererScanner.getDevices();
    
    for (auto& renderer : devs)
    {
        auto dev = response->add_devices();
        dev->set_name(renderer.second->m_FriendlyName);
        dev->set_udn(renderer.second->m_UDN);
    }
    
    log::info("Get renderers returned %d renderers", devs.size());
}
    
void ControlPoint::GetServers(RpcController* controller, const proto::Void* request, proto::DeviceResponse* response, Closure* done)
{
    log::info("Get servers");
    
    auto devs = m_ServerScanner.getDevices();
    
    for (auto& server : devs)
    {
        auto dev = response->add_devices();
        dev->set_name(server.second->m_FriendlyName);
        dev->set_udn(server.second->m_UDN);
    }
    
    log::info("Get servers returned %d servers", devs.size());
}
    
void ControlPoint::Browse(RpcController* controller, const proto::BrowseRequest* request, proto::BrowseResponse* response, Closure* done)
{
    log::info("browse %s %s", request->udn(), request->containerid());

    m_MediaServer.setDevice(m_ServerScanner.getDevice(request->udn()));
    auto items = m_MediaServer.getAllInContainer(request->containerid());
    
    for (auto& item : items)
    {
        auto i = response->add_items();
    
        i->set_id(item->getObjectId());
        i->set_title(item->getTitle());
        i->set_itemclass(convertClass(item->getClass()));
        
        std::string url;
        if (item->getClass() == upnp::Class::AudioContainer)
        {
            url = item->getMetaData(upnp::Property::AlbumArt);
        }
        
        if (url.empty())
        {
            url = item->getAlbumArtUri(upnp::dlna::ProfileId::JpegThumbnail);
        }

        if (!url.empty())
        {
            i->set_thumbnailurl(url);
        }
    }
}
    
void ControlPoint::Play(RpcController* controller, const ::doozy::proto::PlayRequest* request, proto::Void* response, Closure* done)
{
    log::info("play %s %s %s", request->rendererudn(), request->serverudn(), request->containerid());

    m_Cp.setRendererDevice(m_RendererScanner.getDevice(request->rendererudn()));
    m_MediaServer.setDevice(m_ServerScanner.getDevice(request->serverudn()));
    m_Cp.playItems(m_MediaServer, m_MediaServer.getItemsInContainer(request->containerid()));
}
    
void ControlPoint::GetRendererStatus(RpcController* controller, const proto::Device* dev, proto::RendererStatus* status, Closure* done)
{
    upnp::MediaRenderer renderer(m_Client);
    renderer.setDevice(m_RendererScanner.getDevice(dev->udn()));
    
    auto item = renderer.getCurrentTrackInfo();
    for (auto& action : renderer.getAvailableActions())
    {
        status->add_availableactions(convertAction(action));
    }
    
    status->set_title(item->getTitle());
    status->set_artist(item->getMetaData(upnp::Property::Artist));
}
    
}
