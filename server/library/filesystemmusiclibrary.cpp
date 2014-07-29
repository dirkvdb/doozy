//    Copyright (C) 2009 Dirk Vanden Boer <dirk.vdb@gmail.com>
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

#include "filesystemmusiclibrary.h"

#include <stdexcept>
#include <cassert>

#include "serversettings.h"
#include "scanner.h"
#include "utils/log.h"
#include "utils/trace.h"

#include "upnp/upnpitem.h"

using namespace utils;

namespace doozy
{

FilesystemMusicLibrary::FilesystemMusicLibrary(const ServerSettings& settings)
: m_db(settings.getDatabaseFilePath())
, m_destroy(false)
, m_settings(settings)
{
    utils::trace("Create FilesystemMusicLibrary");
}

FilesystemMusicLibrary::~FilesystemMusicLibrary()
{
    m_destroy = true;
    cancelScanThread();
}

void FilesystemMusicLibrary::cancelScanThread()
{
    if (m_scannerThread.joinable())
    {
        {
            std::lock_guard<std::mutex> lock(m_scanMutex);
            if (m_scanner.get())
            {
                m_scanner->cancel();
            }
        }

        m_scannerThread.join();
    }
}

uint32_t FilesystemMusicLibrary::getObjectCount()
{
    return m_db.getObjectCount();
}

uint32_t FilesystemMusicLibrary::getObjectCountInContainer(const std::string& id)
{
    return m_db.getChildCount(id);
}

upnp::ItemPtr FilesystemMusicLibrary::getItem(const std::string& id)
{
    return m_db.getItem(id);
}

std::vector<upnp::ItemPtr> FilesystemMusicLibrary::getItems(const std::string& parentId, uint32_t count, uint32_t offset)
{
    return m_db.getItems(parentId, count, offset);
}

void FilesystemMusicLibrary::scan(bool startFresh)
{
    auto libraryPath = m_settings.getLibraryPath();

    if (startFresh || (m_libraryPath != libraryPath && !m_libraryPath.empty()))
    {
        m_db.clearDatabase();
    }

    m_libraryPath = libraryPath;

    cancelScanThread();
    m_scannerThread = std::thread(&FilesystemMusicLibrary::scannerThread, this);
}

//void FilesystemMusicLibrary::search(const std::string& searchString, utils::ISubscriber<const Track&>& trackSubscriber, utils::ISubscriber<const Album&>& albumSubscriber)
//{
//    m_Db.searchLibrary(searchString, trackSubscriber, albumSubscriber);
//}

void FilesystemMusicLibrary::scannerThread()
{
    try
    {
        auto filenames = m_settings.getAlbumArtFilenames();

        {
            std::lock_guard<std::mutex> lock(m_scanMutex);
            m_scanner.reset(new Scanner(m_db, filenames));
        }
        m_scanner->performScan(m_libraryPath);

        if (!m_destroy)
        {
            m_db.removeNonExistingFiles();
        }
    }
    catch (std::exception& e)
    {
        log::error("Failed to scan library: %s", e.what());
    }

    if (OnScanComplete)
    {
        OnScanComplete();
    }
}

}
