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

#include "common/settings.h"
#include "scanner.h"
#include "utils/log.h"
#include "utils/trace.h"

using namespace utils;

namespace doozy
{

FilesystemMusicLibrary::FilesystemMusicLibrary(const Settings& settings)
: m_Db(settings.get("DBFile"))
, m_Destroy(false)
, m_Settings(settings)
{
    utils::trace("Create FilesystemMusicLibrary");
}

FilesystemMusicLibrary::~FilesystemMusicLibrary()
{
    m_Destroy = true;
    cancelScanThread();
}

void FilesystemMusicLibrary::cancelScanThread()
{
    if (m_ScannerThread.joinable())
    {
        {
            std::lock_guard<std::mutex> lock(m_ScanMutex);
            if (m_Scanner.get())
            {
                m_Scanner->cancel();
            }
        }

        m_ScannerThread.join();
    }
}

uint32_t FilesystemMusicLibrary::getObjectCount()
{
    return m_Db.getObjectCount();
}

uint32_t FilesystemMusicLibrary::getObjectCountInContainer(const std::string& id)
{
    return m_Db.getChildCount(id);
}

upnp::ItemPtr FilesystemMusicLibrary::getItem(const std::string& id)
{
    return m_Db.getItem(id).upnpItem;
}

std::vector<upnp::ItemPtr> FilesystemMusicLibrary::getItems(const std::string& parentId, uint32_t count, uint32_t offset)
{
    auto items = m_Db.getItems(parentId, count, offset);

    std::vector<upnp::ItemPtr> upnpItems;
    std::transform(items.begin(), items.end(), std::back_inserter(upnpItems), [] (const LibraryItem& item) {
        return item.upnpItem;
    });
    
    return upnpItems;
}

void FilesystemMusicLibrary::scan(bool startFresh)
{
    auto libraryPath = m_Settings.get("MusicLibrary");

    if (startFresh || (m_LibraryPath != libraryPath && !m_LibraryPath.empty()))
    {
        m_Db.clearDatabase();
    }

    m_LibraryPath = libraryPath;

    cancelScanThread();
    m_ScannerThread = std::thread(&FilesystemMusicLibrary::scannerThread, this);
}

//void FilesystemMusicLibrary::search(const std::string& searchString, utils::ISubscriber<const Track&>& trackSubscriber, utils::ISubscriber<const Album&>& albumSubscriber)
//{
//    m_Db.searchLibrary(searchString, trackSubscriber, albumSubscriber);
//}

void FilesystemMusicLibrary::scannerThread()
{
    try
    {
        auto filenames = m_Settings.getAsVector("AlbumArtFilenames");

        {
            std::lock_guard<std::mutex> lock(m_ScanMutex);
            m_Scanner.reset(new Scanner(m_Db, filenames));
        }
        m_Scanner->performScan(m_LibraryPath);

        if (!m_Destroy)
        {
            m_Db.removeNonExistingFiles();
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
