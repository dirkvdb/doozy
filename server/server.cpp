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

#include "server.h"

#include "serversettings.h"
#include "mediaserverdevice.h"
#include "common/devicedescriptions.h"

#include "utils/log.h"
#include "utils/readerfactory.h"
#include "utils/stringoperations.h"

#include "upnp/upnpwebserver.h"
#include "upnp/upnphttpreader.h"

#include "library/musiclibraryfactory.h"
#include "library/musiclibraryinterface.h"

using namespace utils;
using namespace utils::stringops;

namespace doozy
{

Server::Server(ServerSettings& settings)
: m_settings(settings)
, m_stop(false)
{
    // make sure we can read http urls
    ReaderFactory::registerBuilder(std::unique_ptr<IReaderBuilder>(new upnp::HttpReaderBuilder()));
}

Server::~Server()
{
    stop();
}

void Server::start()
{
    try
    {
        m_stop = false;
        m_client.initialize();

        auto udn                = "uuid:" + m_settings.getUdn();
        auto friendlyName       = m_settings.getFriendlyName();
        auto description        = format(g_mediaServerDevice.c_str(), m_client.getIpAddress(), m_client.getPort(), friendlyName, udn);
        auto advertiseInterval  = 180;

        log::info("FriendlyName = %s", friendlyName);

        upnp::WebServer webserver("/opt/");
        
        webserver.addVirtualDirectory("Doozy");
        addServiceFileToWebserver(webserver, "ContentDirectoryDesc.xml", g_contentDirectoryService);
        addServiceFileToWebserver(webserver, "ConnectionManagerDesc.xml", g_connectionManagerService);
        //addServiceFileToWebserver(webserver, "AVTransportDesc.xml", g_avTransportService);

        MediaServerDevice dev(udn, description, advertiseInterval, webserver,
                              std::unique_ptr<IMusicLibrary>(MusicLibraryFactory::create(doozy::MusicLibraryType::FileSystem, m_settings)));
        dev.start();

        std::unique_lock<std::mutex> lock(m_mutex);
        m_condition.wait(lock, [this] () { return m_stop == true; });

        dev.stop();
        webserver.removeVirtualDirectory("Doozy");
    }
    catch(std::exception& e)
    {
        log::error(e.what());
    }

    m_client.destroy();
}

void Server::stop()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_stop = true;
    m_condition.notify_all();
}

void Server::addServiceFileToWebserver(upnp::WebServer& webserver, const std::string& filename, const std::string& fileContents)
{
    webserver.addFile("Doozy", filename, "text/xml", fileContents);
}

}
