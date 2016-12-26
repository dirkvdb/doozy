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

#include <algorithm>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

using namespace utils;
using namespace utils::stringops;

namespace doozy
{

struct JsonData
{
    JsonData()
    : writer(sb)
    {
    }

    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer;
};

static const std::string s_okResponse =
    "HTTP/1.1 200 OK\r\n"
    "SERVER: Darwin/15.4.0, UPnP/1.0\r\n"
    "CONTENT-LENGTH: {}\r\n"
    "Access-Control-Allow-Origin: *\r\n"
    "CONTENT-TYPE: text/html\r\n"
    "\r\n"
    "{}";

static const std::string s_badRequestResponse =
    "HTTP/1.1 400 Bad Request\r\n"
    "SERVER: Darwin/15.4.0, UPnP/1.0\r\n"
    "CONTENT-LENGTH: 0\r\n"
    "Access-Control-Allow-Origin: *\r\n"
    "\r\n"
    "{}";

static const std::string s_errorResponse =
    "HTTP/1.1 500 Internal Server Error\r\n"
    "SERVER: Darwin/15.4.0, UPnP/1.0\r\n"
    "CONTENT-LENGTH: 0\r\n"
    "Access-Control-Allow-Origin: *\r\n"
    "\r\n"
    "{}";

ControlPoint::ControlPoint()
: m_client(upnp::factory::createClient(m_io))
, m_cp(*m_client)
, m_rendererScanner(*m_client, upnp::DeviceType(upnp::DeviceType::MediaRenderer, 1))
, m_serverScanner(*m_client, upnp::DeviceType(upnp::DeviceType::MediaServer, 1))
, m_webServer(m_io)
{
    m_webServer.setRequestHandler(upnp::http::Method::Get, [this] (const upnp::http::Request& req, std::function<void(std::string)> cb) {
        return handleRequest(req, cb);
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

static std::string_view getParam(const std::vector<std::pair<std::string, std::string>>& params, std::string_view name)
{
    auto iter = std::find_if(params.begin(), params.end(), [name] (const auto& param) {
        return param.first == name;
    });

    if (iter == params.end())
    {
        throw std::invalid_argument("Param not found: " + name.to_string());
    }

    log::info(iter->second);
    return iter->second;
}

static std::string getDevices(const upnp::DeviceScanner& scanner)
{
    const auto devs = scanner.getDevices();

    rapidjson::StringBuffer s;
    rapidjson::Writer<rapidjson::StringBuffer> writer(s);

    writer.StartObject();
    writer.Key("devices");
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

bool ControlPoint::handleRequest(const upnp::http::Request& req, std::function<void(std::string)> cb)
{
    log::info("Request: {}", req.url());

    try
    {
        if (req.url() == "/servers")
        {
            auto devs = getDevices(m_serverScanner);
            cb(fmt::format(s_okResponse, devs.size(), devs));
        }
        else if (req.url() == "/renderers")
        {
            auto devs = getDevices(m_rendererScanner);
            cb(fmt::format(s_okResponse, devs.size(), devs));
        }
        else if (utils::stringops::startsWith(req.url(), "/browse?"))
        {
            auto params = upnp::http::Server::getQueryParameters(req.url());
            browse(getParam(params, "udn"), getParam(params, "id"), cb);
        }
        else if (utils::stringops::startsWith(req.url(), "/rendererstatus?"))
        {
            auto params = upnp::http::Server::getQueryParameters(req.url());
            getRendererStatus(getParam(params, "udn"), cb);
        }
        else if (utils::stringops::startsWith(req.url(), "/play?"))
        {
            auto params = upnp::http::Server::getQueryParameters(req.url());
            play(getParam(params, "rendererudn"),
                 getParam(params, "serverudn"),
                 getParam(params, "id"), cb);
        }
        else
        {
            return false;
        }
    }
    catch (const std::invalid_argument& e)
    {
        log::error(e.what());
        cb(s_badRequestResponse);
    }

    return true;
}

void ControlPoint::browse(std::string_view udn, std::string_view containerId, std::function<void(std::string)> cb)
{
    auto s = udn.to_string();
    log::debug("browse {} {}", udn.to_string(), containerId.to_string());

    auto mediaServer = std::make_shared<upnp::MediaServer>(*m_client);
    auto dev = m_serverScanner.getDevice(udn);
    if (!dev)
    {
        cb(s_badRequestResponse);
        return;
    }

    mediaServer->setDevice(dev, [this, cb, mediaServer, id = containerId.to_string()] (upnp::Status s) {
        if (!s)
        {
            cb(s_errorResponse);
            return;
        }

        auto jsonData = std::make_shared<JsonData>();
        jsonData->writer.StartObject();
        jsonData->writer.Key("items");
        jsonData->writer.StartArray();

        mediaServer->getAllInContainer(id, [jsonData, mediaServer, cb] (upnp::Status s, const std::vector<upnp::Item>& items) {
            if (!s)
            {
                cb(s_errorResponse);
                return;
            }

            if (items.empty())
            {
                jsonData->writer.EndArray();
                jsonData->writer.EndObject();

                assert(jsonData->writer.IsComplete());

                cb(fmt::format(s_okResponse, jsonData->sb.GetSize(), jsonData->sb.GetString()));
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
                        std::function<void(std::string)> cb)
{
    log::info("play {} {} {}", rendererUdn.data(), serverUdn.data(), containerId.data());

    auto rendererDev = m_rendererScanner.getDevice(rendererUdn);
    auto serverDev = m_serverScanner.getDevice(serverUdn);
    if (!rendererDev || !serverDev)
    {
        cb(s_badRequestResponse);
        return;
    }

    m_cp.setRendererDevice(rendererDev, [this, cb, id = containerId.to_string(), serverDev] (upnp::Status s) {
        if (!s)
        {
            log::error("Failed to set renderer device: {}", s.what());
            cb(s_errorResponse);
            return;
        }

        auto mediaServer = std::make_shared<upnp::MediaServer>(*m_client);
        mediaServer->setDevice(serverDev, [=] (upnp::Status s) {
            if (!s)
            {
                log::error("Failed to set server device: {}", s.what());
                cb(s_errorResponse);
                return;
            }

            auto playListItems = std::make_shared<std::vector<upnp::Item>>();
            mediaServer->getItemsInContainer(id, [=] (const upnp::Status& s, const std::vector<upnp::Item>& items) {
                if (!s)
                {
                    log::error("Failed to obtain items for playback: {}", s.what());
                    cb(s_errorResponse);
                    return;
                }

                if (items.empty())
                {
                    m_cp.playItemsAsPlaylist(*mediaServer, *playListItems, [=] (const upnp::Status& s) {
                        if (!s)
                        {
                            log::error("Failed to play playlist: {}", s.what());
                            cb(s_errorResponse);
                            return;
                        }

                        cb(fmt::format(s_okResponse, 0, ""));
                    });
                }
                else
                {
                    for (auto& item : items)
                    {
                        playListItems->emplace_back(item);
                    }
                }
            });
        });
    });
}

void ControlPoint::getRendererStatus(std::string_view udn, std::function<void(std::string)> cb)
{
    auto renderer = std::make_shared<upnp::MediaRenderer>(*m_client);
    renderer->useDefaultConnection();

    renderer->setDevice(m_rendererScanner.getDevice(udn), [this, renderer, cb] (upnp::Status s) {
        if (!s)
        {
            log::error("Failed to set renderer device: {}", s.what());
            cb(s_errorResponse);
            return;
        }

        renderer->getCurrentTrackInfo([=] (upnp::Status s, const upnp::Item& item) {
            if (!s)
            {
                log::error("Failed to get current track info: {}", s.what());
                cb(s_errorResponse);
                return;
            }

            renderer->getAvailableActions([renderer, cb, item] (upnp::Status s, const std::set<upnp::MediaRenderer::Action>& actions) {
                if (!s)
                {
                    log::error("Failed to get available renderer actions: {}", s.what());
                    cb(s_errorResponse);
                    return;
                }

                JsonData jsonData;
                jsonData.writer.StartObject();

                jsonData.writer.Key("title");
                jsonData.writer.String(item.getTitle());
                jsonData.writer.Key("artist");
                jsonData.writer.String(item.getMetaData(upnp::Property::Artist));

                jsonData.writer.Key("actions");
                jsonData.writer.StartArray();
                for (auto& action : actions)
                {
                    jsonData.writer.String(upnp::MediaRenderer::actionToString(action));
                }
                jsonData.writer.EndArray();

                jsonData.writer.EndObject();
                assert(jsonData.writer.IsComplete());

                cb(fmt::format(s_okResponse, jsonData.sb.GetSize(), jsonData.sb.GetString()));
            });
        });
    });
}
    
}
