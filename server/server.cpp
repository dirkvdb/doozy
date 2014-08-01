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
#include "filerequesthandler.h"
#include "common/devicedescriptions.h"

#include "utils/log.h"
#include "utils/readerfactory.h"
#include "utils/stringoperations.h"

#include "upnp/upnpwebserver.h"
#include "upnp/upnphttpreader.h"

#include "library/filesystemmusiclibrary.h"
#include "library/musicdb.h"

using namespace utils;
using namespace utils::stringops;

namespace doozy
{

static const std::string g_mediaDir = "Media";

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

        fileops::createDirectoryIfNotExists(m_settings.getCachePath());
        upnp::WebServer webserver(m_settings.getCachePath());
        
        webserver.addVirtualDirectory("Doozy");
        addServiceFileToWebserver(webserver, "ContentDirectoryDesc.xml", g_contentDirectoryService);
        addServiceFileToWebserver(webserver, "ConnectionManagerDesc.xml", g_connectionManagerService);
        //addServiceFileToWebserver(webserver, "AVTransportDesc.xml", g_avTransportService);

        auto getInfoCb = [this] (const std::string& path) -> fileops::FileSystemEntryInfo {
            MusicDb musicDb(m_settings.getDatabaseFilePath());
            return fileops::getFileInfo(musicDb.getItemPath(fileops::getFileNameWithoutExtension(path)));
        };
        
        auto requestCb = [this] (const std::string& path) {
            return std::make_shared<FileRequestHandler>(m_settings.getDatabaseFilePath(), path);
        };
        
        webserver.addVirtualDirectory(g_mediaDir, getInfoCb, requestCb);

        auto library = std::unique_ptr<IMusicLibrary>(new FilesystemMusicLibrary(m_settings, webserver.getWebRootUrl()));
        MediaServerDevice dev(udn, description, advertiseInterval, std::move(library));
        dev.start();

        std::unique_lock<std::mutex> lock(m_mutex);
        m_condition.wait(lock, [this] () { return m_stop == true; });

        dev.stop();
        webserver.removeVirtualDirectory("Doozy");
        webserver.removeVirtualDirectory(g_mediaDir);
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
