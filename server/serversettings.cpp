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

#include "serversettings.h"
#include "utils/log.h"
#include "utils/fileoperations.h"
#include "utilsconfig.h"

#ifndef HAVE_XDGBASEDIR
static_assert(false, "Utils not compiled with xdg-basedir support");
#endif

namespace doozy
{

using namespace utils;

static const std::string g_friendlyName = "FriendlyName";
static const std::string g_udn          = "UDN";
static const std::string g_dbFilePath   = "DBFile";
static const std::string g_libraryPath  = "MusicLibrary";
static const std::string g_dataPath     = "DataDirectory";
static const std::string g_artNames     = "AlbumArtFilenames";

void ServerSettings::loadFromFile(const std::string& filepath)
{
    auto path = filepath;
    if (path.empty())
    {
        path = fileops::combinePath(fileops::getConfigDirectory(), "doozyserver.cfg");
    }
    
    log::info("Using config file: {}", path);
    m_settings.loadFromFile(path);
}

std::string  ServerSettings::getFriendlyName() const
{
    return m_settings.get(g_friendlyName, "Doozy");
}

std::string  ServerSettings::getUdn() const
{
    return m_settings.get(g_udn, "356a6e90-8e58-11e2-9e96-0800200c9a66");
}

std::string  ServerSettings::getDatabaseFilePath() const
{
    return m_settings.get(g_dbFilePath, fileops::combinePath(getDataPath(), "doozyserver.db"));
}

std::string  ServerSettings::getLibraryPath() const
{
    return m_settings.get(g_libraryPath, "/Volumes/Data/Music");
}

std::string  ServerSettings::getDataPath() const
{
    auto configDir = fileops::getDataDirectory();
    return m_settings.get(g_dataPath, configDir);
}

std::string  ServerSettings::getCachePath() const
{
    return fileops::combinePath(getDataPath(), "ArtCache");
}

std::vector<std::string> ServerSettings::getAlbumArtFilenames() const
{
    return m_settings.getAsVector(g_artNames, { "cover.jpg" });
}

}
