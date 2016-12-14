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

#include "upnp/upnp.factory.h"
#include "upnp/stringview.h"
#include "upnp/upnp.mediaserver.h"

#include "utils/stringoperations.h"

#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

using namespace utils;
using namespace utils::stringops;

namespace doozy
{

ControlPoint::ControlPoint()
: m_client(upnp::factory::createClient(m_io))
, m_cp(*m_client)
, m_rendererScanner(*m_client, upnp::DeviceType(upnp::DeviceType::MediaRenderer, 1))
, m_serverScanner(*m_client, upnp::DeviceType(upnp::DeviceType::MediaServer, 1))
, m_webServer(m_io)
{
    m_webServer.setRequestHandler(upnp::http::Method::Get, [this] (const upnp::http::Request& req, std::function<void(upnp::http::StatusCode, std::string)> cb) {
        handleRequest(req, cb);
    });
}

void ControlPoint::start(const std::string& networkInterface)
{
    try
    {
        m_client->initialize();
        m_serverScanner.start();
        m_serverScanner.refresh();
        m_rendererScanner.start();
        m_rendererScanner.refresh();

        m_webServer.start(asio::ip::tcp::endpoint(asio::ip::address_v4::any(), 4444));

        m_cp.setWebserver(m_webServer);
        m_cp.activate([] (const upnp::Status& s) {
            if (!s)
            {
                log::error("Failed to activate controlpoint: {}", s.what());
            }
        });
        
        // log::info("Webserver listening url: %s", m_webServer->getWebRootUrl());

        m_io.run();
    }
    catch(std::exception& e)
    {
        log::error(e.what());
    }
}
    
void ControlPoint::stop()
{
    m_cp.deactivate([] (const upnp::Status& s) {
        if (!s)
        {
            log::warn("Failed to deactivate controlpoint: {}", s.what());
        }
    });

    m_webServer.stop();
    m_rendererScanner.stop();
    m_serverScanner.stop();
    m_client->uninitialize();
}

static std::string getDevices(const upnp::DeviceScanner& scanner, const char* name)
{
    const auto devs = scanner.getDevices();

    rapidjson::StringBuffer s;
    rapidjson::Writer<rapidjson::StringBuffer> writer(s);

    writer.StartObject();
    writer.Key(name);
    writer.StartArray();
    for (auto& dev : devs)
    {
        writer.StartObject();
        writer.Key("name");
        writer.String(dev->friendlyName);
        writer.Key("udn");
        writer.String(dev->udn);
        writer.EndObject();
    }

    writer.EndArray();
    writer.EndObject();

    assert(writer.IsComplete());

    return s.GetString();
}

void ControlPoint::handleRequest(const upnp::http::Request& req, std::function<void(upnp::http::StatusCode, std::string)> cb)
{
    static const std::string okResponse =
        "HTTP/1.1 200 OK\r\n"
        "SERVER: Darwin/15.4.0, UPnP/1.0\r\n"
        "CONTENT-LENGTH: {}\r\n"
        "CONTENT-TYPE: text/html\r\n"
        "\r\n"
        "{}";

    log::info("Request: {}", req.url());
    std::string response;

    if (req.url() == "/getservers")
    {
        cb(upnp::http::StatusCode::Ok, getDevices(m_serverScanner, "servers"));
    }
    else if (req.url() == "/getrenderers")
    {
        cb(upnp::http::StatusCode::Ok, getDevices(m_rendererScanner, "renderers"));
    }
    else if (utils::stringops::startsWith(req.url(), "/browse"))
    {
        auto params = upnp::http::Server::getQueryParameters(req.url());
        if (params.size() != 2)
        {
            cb(upnp::http::StatusCode::BadRequest, "");
            return;
        }

        browse(params[0].second, params[1].second, cb);
    }
    else
    {
        cb(upnp::http::StatusCode::BadRequest, "");
    }
}

void ControlPoint::browse(std::string_view udn, std::string_view containerId, std::function<void(upnp::http::StatusCode, std::string)> cb)
{
    log::debug("browse {} {}", udn, containerId);

    auto mediaServer = std::make_shared<upnp::MediaServer>(*m_client);
    mediaServer->setDevice(m_serverScanner.getDevice(udn), [this, cb, mediaServer, id = containerId.to_string()] (upnp::Status s) {
        if (!s)
        {
            cb(upnp::http::StatusCode::InternalServerError, "");
            return;
        }

        struct JsonData
        {
            JsonData()
            : writer(sb)
            {
            }

            rapidjson::StringBuffer sb;
            rapidjson::Writer<rapidjson::StringBuffer> writer;
        };

        auto jsonData = std::make_shared<JsonData>();
        jsonData->writer.StartObject();
        jsonData->writer.Key("items");
        jsonData->writer.StartArray();

        mediaServer->getAllInContainer(id, [jsonData, cb] (upnp::Status s, const std::vector<upnp::Item>& items) {
            if (!s)
            {
                cb(upnp::http::StatusCode::InternalServerError, "");
                return;
            }

            if (items.empty())
            {
                jsonData->writer.EndArray();
                jsonData->writer.EndObject();

                assert(jsonData->writer.IsComplete());

                cb(upnp::http::StatusCode::Ok, jsonData->sb.GetString());
                return;
            }

            for (auto& item : items)
            {
                jsonData->writer.StartObject();

                jsonData->writer.Key("id");
                jsonData->writer.String(item.getObjectId());

                jsonData->writer.Key("title");
                jsonData->writer.String(item.getTitle());

                jsonData->writer.Key("class");
                jsonData->writer.String(item.getClassString());

                std::string url;
                if (item.getClass() == upnp::Class::AudioContainer)
                {
                    url = item.getMetaData(upnp::Property::AlbumArt);
                }

                if (url.empty())
                {
                    url = item.getAlbumArtUri(upnp::dlna::ProfileId::JpegThumbnail);
                }

                if (!url.empty())
                {
                    jsonData->writer.Key("thumbnailurl");
                    jsonData->writer.String(url);
                }

                jsonData->writer.EndObject();
            }
        });
    });
}

void ControlPoint::play(std::string_view rendererUdn,
                        std::string_view serverUdn,
                        std::string_view containerId,
                        std::function<void(upnp::http::StatusCode, std::string)> cb)
{
    log::info("play {} {} {}", rendererUdn, serverUdn, containerId);

    m_cp.setRendererDevice(m_rendererScanner.getDevice(rendererUdn), [this, cb, id = containerId.to_string(), server = serverUdn.to_string()] (upnp::Status s) {
        if (!s)
        {
            cb(upnp::http::StatusCode::InternalServerError, "");
            return;
        }

        auto mediaServer = std::make_shared<upnp::MediaServer>(*m_client);
        mediaServer->setDevice(m_serverScanner.getDevice(server), [=] (upnp::Status s) {
            if (!s)
            {
                cb(upnp::http::StatusCode::InternalServerError, "");
                return;
            }

            mediaServer->getItemsInContainer(id, [=] (upnp::Status s, const std::vector<upnp::Item>& items) {
                if (!s)
                {
                    cb(upnp::http::StatusCode::InternalServerError, "");
                    return;
                }

                m_cp.playItemsAsPlaylist(*mediaServer, items, [=] (upnp::Status s) {
                    if (!s)
                    {
                        cb(upnp::http::StatusCode::InternalServerError, "");
                        return;
                    }

                    cb(upnp::http::StatusCode::Ok, "");
                });
            });
        });
    });
}

//static rpc::Action::type convertAction(upnp::MediaRenderer::Action action)
//{
//    switch (action)
//    {
//        case upnp::MediaRenderer::Action::Play:     return rpc::Action::Play;
//        case upnp::MediaRenderer::Action::Next:     return rpc::Action::Next;
//        case upnp::MediaRenderer::Action::Previous: return rpc::Action::Previous;
//        case upnp::MediaRenderer::Action::Seek:     return rpc::Action::Seek;
//        case upnp::MediaRenderer::Action::Stop:     return rpc::Action::Stop;
//        case upnp::MediaRenderer::Action::Pause:    return rpc::Action::Pause;
//        default: return rpc::Action::Stop; // huh? :-)
//    }
//}

//void ControlPoint::GetRendererStatus(doozy::rpc::RendererStatus& status, const doozy::rpc::Device& dev)
//{
//    upnp::MediaRenderer renderer(m_Client);
//    renderer.setDevice(m_RendererScanner.getDevice(dev.udn));
    
//    auto item = renderer.getCurrentTrackInfo();
//    auto actions = renderer.getAvailableActions();
//    std::transform(actions.begin(), actions.end(), status.availableActions.begin(), [] (upnp::MediaRenderer::Action a) {
//        return convertAction(a);
//    });
    
//    status.title = item->getTitle();
//    status.artist = item->getMetaData(upnp::Property::Artist);
//}
    
}
