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


#pragma once

#include <iostream>
#include <condition_variable>
#include <mutex>

#include "upnp/asio.h"
#include "upnp/upnp.clientinterface.h"
#include "upnp/upnp.http.server.h"
#include "upnp/upnp.devicescanner.h"
#include "upnp/upnp.controlpoint.h"
#include "upnp/upnp.http.types.h"

#include "common/doozydeviceinterface.h"

namespace doozy
{

class ControlPoint : public IDevice
{
public:
    ControlPoint();

    void start(const std::string& networkInterface) override;
    void stop() override;

private:
    void handleRequest(const upnp::http::Request& req, std::function<void(upnp::http::StatusCode, std::string)> cb);

    void browse(std::string_view udn,
                std::string_view containerId,
                std::function<void(upnp::http::StatusCode, std::string)> cb);

    void play(std::string_view rendererUdn,
              std::string_view serverUdn,
              std::string_view containerId,
              std::function<void(upnp::http::StatusCode, std::string)> cb);

    void getRendererStatus(std::string_view udn,
                           std::function<void(upnp::http::StatusCode, std::string)> cb);

    asio::io_service                    m_io;
    std::unique_ptr<upnp::IClient>      m_client;
    upnp::ControlPoint                  m_cp;
    upnp::DeviceScanner                 m_rendererScanner;
    upnp::DeviceScanner                 m_serverScanner;
    upnp::http::Server                  m_webServer;
};

}

