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

#include "scanner.h"

#include <cassert>
#include <ctime>

#include "musicdb.h"
#include "utils/stringoperations.h"
#include "utils/fileoperations.h"
#include "utils/log.h"
#include "subscribers.h"
#include "doozyconfig.h"

#include "audio/audiometadata.h"
#include "image/imagefactory.h"
#include "image/imageloadstoreinterface.h"
#include "mimetypes.h"

#include "upnp/upnptypes.h"
#include "upnp/upnpitem.h"

using namespace utils;
using namespace fileops;

namespace doozy
{

//static constexpr int32_t ALBUM_ART_DB_SIZE = 96;
static const std::string g_unknownAlbum = "Unknown Album";
static const std::string g_unknownArtist = "Unknown Artist";
static const std::string g_unknownTitle = "Unknown Title";
static const std::string g_variousArtists = "Various Artists";

static const std::string g_rootId = "0";
static const std::string g_browseFileSystemId = "@1";

Scanner::Scanner(MusicDb& db, const std::vector<std::string>& albumArtFilenames)
: m_LibraryDb(db)
, m_ScannedFiles(0)
, m_AlbumArtFilenames(albumArtFilenames)
, m_InitialScan(false)
, m_Stop(false)
{
}

Scanner::~Scanner()
{
    cancel();
}

void Scanner::performScan(const std::string& libraryPath)
{
    m_Stop = false;

    time_t startTime = time(nullptr);
    log::debug("Starting library scan in: %s", libraryPath);

    m_InitialScan = m_LibraryDb.getObjectCount() == 0;
    m_ScannedFiles = 0;
    
    if (m_InitialScan)
    {
        createInitialLayout();
    }

    scan(libraryPath, g_browseFileSystemId);
    log::debug("Library scan took %d seconds. Scanned %d files.", time(nullptr) - startTime, m_ScannedFiles);
}

void Scanner::createInitialLayout()
{
    // Root Item
    LibraryItem root;
    root.name = PACKAGE_NAME;
    root.objectId = g_rootId;
    root.title = PACKAGE_NAME;
    root.parentId = "-1";
    root.upnpClass = toString(upnp::Class::Container);
    m_LibraryDb.addItem(root);
    
    // Browse folders
    LibraryItem browse;
    browse.name = "Browse filesystem";
    browse.objectId = g_browseFileSystemId;
    browse.title = "Browse filesystem";
    browse.parentId = g_rootId;
    browse.upnpClass = toString(upnp::Class::StorageFolder);
    m_LibraryDb.addItem(browse);
}

void Scanner::scan(const std::string& dir, const std::string& parentId)
{
    std::vector<LibraryItem> items;

    uint32_t index = 0;
    for (auto& entry : Directory(dir))
    {
        if (m_Stop)
        {
            break;
        }
        
        auto type = entry.type();
        if (type == FileSystemEntryType::Directory)
        {
            auto path = entry.path();
            std::string objectId;
            if (!m_LibraryDb.itemExists(entry.path(), objectId))
            {
                // TODO: make sure this id is unique!!
                objectId = stringops::format("%s@%d", parentId, index++);
            
                LibraryItem item;
                item.path = entry.path();
                item.name = fileops::getFileName(item.path);
                item.objectId = objectId;
                item.title = item.name;
                item.parentId = parentId;
                item.upnpClass = toString(upnp::Class::Container);

                m_LibraryDb.addItem(item);
                log::debug("Add container: %s (%s) parent: %s", entry.path(), objectId, parentId);
            }

            scan(path, objectId);
        }
        else if (type == FileSystemEntryType::File)
        {
            try
            {
                auto path = entry.path();
                onFile(path, index, parentId, items);
                ++index;
            }
            catch (std::exception& e)
            {
                log::error(e.what());
            }
        }
    }

    if (!items.empty())
    {
        m_LibraryDb.addItems(items);
    }
}

void Scanner::cancel()
{
    m_Stop = true;
}

void Scanner::onFile(const std::string& filepath, uint32_t index, const std::string& parentId, std::vector<LibraryItem>& items)
{
    ++m_ScannedFiles;

    auto type = mime::groupFromFile(filepath);
    if (type == mime::Group::Other)
    {
        return;
    }

    auto info = getFileInfo(filepath);
    auto status = m_LibraryDb.getItemStatus(filepath, info.modifyTime);

    if (status == ItemStatus::UpToDate)
    {
        return;
    }

    auto id = stringops::format("%s@%d", parentId, index);

    LibraryItem item;
    item.path = filepath;
    item.name = fileops::getFileName(item.path);
    item.modifiedTime = info.modifyTime;
    item.fileSize = info.sizeInBytes;
    
    item.objectId = id;
    item.parentId = parentId;

    switch (type)
    {
        case mime::Group::Audio:
            item.upnpClass = toString(upnp::Class::Audio);
            break;
        case mime::Group::Video:
            item.upnpClass = toString(upnp::Class::Video);
            break;
        case mime::Group::Image:
            item.upnpClass = toString(upnp::Class::Image);
            break;
        default:
            throw std::runtime_error("Unexpected mime type");
    }

    if (type == mime::Group::Audio)
    {
        try
        {
            audio::Metadata md(filepath, audio::Metadata::ReadAudioProperties::Yes);
            item.artist         = md.getArtist();
            item.title          = md.getTitle();
            item.album          = md.getAlbum();
            item.genre          = md.getGenre();
            item.date           = std::to_string(md.getYear());
            item.trackNumber    = md.getTrackNr();
        }
        catch (std::exception& e)
        {
            log::warn(e.what());
        }
        
    }
    
//    track.albumArtist   = md.getAlbumArtist();
//    track.composer      = md.getComposer();
//    track.discNr        = md.getDiscNr();
//    track.bitrate       = md.getBitRate();
//    track.sampleRate    = md.getSampleRate();
//    track.channels      = md.getChannels();
//    track.durationInSec = md.getDuration();

//    Album album;
//    std::string albumId;
//    m_LibraryDb.albumExists(track.album, albumId);
//    if (!albumId.empty())
//    {
//        album = m_LibraryDb.getAlbum(albumId);
//    }
//
//    if (albumId.empty() || ((!track.albumArtist.empty()) && track.albumArtist != album.artist))
//    {
//        album.title         = track.album;
//        album.artist        = track.albumArtist.empty() ? track.artist : track.albumArtist;
//        album.year          = track.year;
//        album.genre         = track.genre;
//        album.durationInSec = track.durationInSec;
//        album.dateAdded     = m_InitialScan ? track.modifiedTime : time(nullptr);
//
//        AlbumArt art(albumId);
//        art.setAlbumArt(md.getAlbumArt());
//        processAlbumArt(filepath, art);
//
//        m_LibraryDb.addAlbum(album, art);
//    }
//    else
//    {
//        if (!track.albumArtist.empty())
//        {
//            //if a track has an album artist set, we trust it is the right one
//            if (track.albumArtist != album.artist)
//            {
//                album.artist = track.albumArtist;
//            }
//        }
//        else
//        {
//            //if no album artist set and we detect different artists for an
//            //album, we set the album artist to VARIOUS_ARTISTS
//            if (album.artist != g_variousArtists && album.artist != track.artist)
//            {
//                album.artist = g_variousArtists;
//            }
//        }
//
//        assert(album.id == albumId);
//
//        AlbumArt art = m_LibraryDb.getAlbumArt(albumId);
//        if (art.getData().empty() || status == MusicDb::NeedsUpdate)
//        {
//            art.setAlbumArt(md.getAlbumArt());
//            processAlbumArt(filepath, art);
//
//            if (art.getDataSize() > 0)
//            {
//                m_LibraryDb.setAlbumArt(albumId, art.getData());
//            }
//        }
//
//        if (status == MusicDb::DoesntExist)
//        {
//            album.durationInSec += track.durationInSec;
//        }
//
//        if (album.genre.empty())
//        {
//            album.genre = track.genre;
//        }
//
//        m_LibraryDb.updateAlbum(album);
//    }
//
//    track.albumId = album.id;
//    if (status == MusicDb::DoesntExist)
//    {
//        log::debug("New track: %s %s", filepath, track.albumId);
//        //m_LibraryDb.addTrack(track);
//    }
//    else if (status == MusicDb::NeedsUpdate)
//    {
//        log::debug("Needs update: %s", filepath);
//        //m_LibraryDb.updateTrack(track);
//    }

    items.push_back(item);
    log::debug("Add Item: %s parent: %s (%d)", filepath, parentId, index);
}

void Scanner::processAlbumArt(const std::string& filepath, AlbumArt& art)
{
//    if (art.getData().empty())
//    {
//        //no embedded album art found, see if we can find a cover.jpg, ... file
//        auto dir = fileops::getPathFromFilepath(filepath);
//
//        for (auto& filename : m_AlbumArtFilenames)
//        {
//            try
//            {
//                auto possibleAlbumArt = fileops::combinePath(dir, filename);
//                audio::Metadata::AlbumArt artData;
//                artData.data = fileops::readFile(possibleAlbumArt);
//                log::debug("Art found in: %s", possibleAlbumArt);
//
//                art.setAlbumArt(std::move(artData));
//            }
//            catch (std::exception&) {}
//        }
//    }
//
//
//    // resize the album art if it is present
//    if (!art.getData().empty())
//    {
//        try
//        {
//            auto image = image::Factory::createFromData(art.getData());
//            image->resize(ALBUM_ART_DB_SIZE, ALBUM_ART_DB_SIZE, image::ResizeAlgorithm::Bilinear);
//
//            auto pngStore = image::Factory::createLoadStore(image::Type::Png);
//            art.getData() = pngStore->storeToMemory(*image);
//        }
//        catch (std::exception& e)
//        {
//            log::warn("Failed to scale image: %s", e.what());
//        }
//    }
}

}
