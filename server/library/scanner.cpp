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
#include "upnp/upnputils.h"

using namespace utils;
using namespace fileops;

namespace doozy
{

static constexpr int32_t ALBUM_ART_CACHE_SIZE = 128;
static const std::string g_unknownAlbum = "Unknown Album";
static const std::string g_unknownArtist = "Unknown Artist";
static const std::string g_unknownTitle = "Unknown Title";
static const std::string g_variousArtists = "Various Artists";

static const std::string g_rootId = "0";
static const std::string g_musicId = "0@1";
static const std::string g_browseFileSystemId = "0@2";

Scanner::Scanner(MusicDb& db, const std::vector<std::string>& albumArtFilenames, const std::string& cacheDir)
: m_libraryDb(db)
, m_cacheDir(cacheDir)
, m_scannedFiles(0)
, m_albumArtFilenames(albumArtFilenames)
, m_jpgLoadStore(image::Factory::createLoadStore(image::Type::Jpeg))
, m_pngLoadStore(image::Factory::createLoadStore(image::Type::Png))
, m_initialScan(false)
, m_stop(false)
{
}

Scanner::~Scanner()
{
    cancel();
}

void Scanner::performScan(const std::string& libraryPath)
{
    m_stop = false;

    time_t startTime = time(nullptr);
    log::info("Starting library scan in: %s", libraryPath);

    m_initialScan = m_libraryDb.getObjectCount() == 0;
    m_scannedFiles = 0;
    
    if (m_initialScan)
    {
        createInitialLayout();
    }
    
    scan(libraryPath, g_browseFileSystemId);
    log::info("Library scan took %d seconds. Scanned %d files.", time(nullptr) - startTime, m_scannedFiles);
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
    m_libraryDb.addItem(root);

    // Music
    LibraryItem music;
    music.name = "Music";
    music.objectId = g_musicId;
    music.title = "Music";
    music.parentId = g_rootId;
    music.upnpClass = toString(upnp::Class::Container);
    m_libraryDb.addItem(music);

    // Music -> Albums
    LibraryItem albums;
    albums.name = "Albums";
    albums.objectId = g_musicId + "@1";
    albums.title = "Albums";
    albums.parentId = music.objectId;
    albums.upnpClass = toString(upnp::Class::Container);
    m_libraryDb.addItem(albums);

    // Browse folders
    LibraryItem browse;
    browse.name = "Browse filesystem";
    browse.objectId = g_browseFileSystemId;
    browse.title = "Browse filesystem";
    browse.parentId = g_rootId;
    browse.upnpClass = toString(upnp::Class::StorageFolder);
    m_libraryDb.addItem(browse);
}

void Scanner::scan(const std::string& dir, const std::string& parentId)
{
    std::vector<LibraryItem> items;

    auto id = m_libraryDb.getUniqueIdInContainer(parentId);

    for (auto& entry : Directory(dir))
    {
        if (m_stop)
        {
            break;
        }

        auto type = entry.type();
        if (type == FileSystemEntryType::Directory)
        {
            auto path = entry.path();
            std::string objectId;
            if (!m_libraryDb.itemExists(entry.path(), objectId))
            {
                objectId = stringops::format("%s@%d", parentId, id++);

                LibraryItem item;
                item.path = entry.path();
                item.name = fileops::getFileName(item.path);
                item.objectId = objectId;
                item.title = item.name;
                item.parentId = parentId;
                item.upnpClass = toString(upnp::Class::Container);

                if (checkAlbumArt(item.path, item.objectId))
                {
                    item.thumbnail = item.objectId + "_thumb.jpg";
                }

                m_libraryDb.addItem(item);
                log::debug("Add container: %s (%s) parent: %s", entry.path(), objectId, parentId);
            }

            scan(path, objectId);
        }
        else if (type == FileSystemEntryType::File)
        {
            try
            {
                auto path = entry.path();
                onFile(path, id++, parentId, items);
            }
            catch (std::exception& e)
            {
                --id;
                log::error(e.what());
            }
        }
    }

    if (!items.empty())
    {
        m_libraryDb.addItems(items);
    }
}

void Scanner::cancel()
{
    m_stop = true;
}

void Scanner::onFile(const std::string& filepath, uint64_t id, const std::string& parentId, std::vector<LibraryItem>& items)
{
    ++m_scannedFiles;

    auto type = mime::groupFromFile(filepath);
    if (type == mime::Group::Other)
    {
        return;
    }

    auto info = getFileInfo(filepath);
    auto status = m_libraryDb.getItemStatus(filepath, info.modifyTime);

    if (status == ItemStatus::UpToDate)
    {
        return;
    }

    LibraryItem item;
    item.path = filepath;
    item.name = fileops::getFileName(item.path);
    item.modifiedTime = info.modifyTime;
    item.fileSize = info.sizeInBytes;
    item.mimeType = toString(mime::typeFromFile(item.path));
    
    item.objectId = stringops::format("%s@%d", parentId, id);
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
            item.duration       = md.getDuration();
            item.nrChannels     = md.getChannels();
            item.sampleRate     = md.getSampleRate();
            item.bitrate        = md.getBitRate();
            
            if (processAlbumArt(filepath, item.objectId, md.getAlbumArt()))
            {
                item.thumbnail = item.objectId + "_thumb.jpg";
            }
        }
        catch (std::exception& e)
        {
            log::warn(e.what());
        }
    }
    
//    track.albumArtist   = md.getAlbumArtist();
//    track.composer      = md.getComposer();
//    track.discNr        = md.getDiscNr();

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
    log::debug("Add Item: %s (%s parent: %s)", filepath, item.objectId, parentId);
}

bool Scanner::checkAlbumArt(const std::string& directoryPath, const std::string& id)
{
    for (auto& artName : m_albumArtFilenames)
    {
        try
        {
            auto possibleAlbumArt = fileops::combinePath(directoryPath, artName);
            if (fileops::pathExists(possibleAlbumArt))
            {
                auto image = image::Factory::createFromUri(possibleAlbumArt);
                image->resize(ALBUM_ART_CACHE_SIZE, ALBUM_ART_CACHE_SIZE, image::ResizeAlgorithm::Bilinear);
                m_jpgLoadStore->storeToFile(*image, fileops::combinePath(m_cacheDir, id + "_thumb.jpg"));
                return true;
            }
        }
        catch (std::exception&) {}
    }

    return false;
}

bool Scanner::processAlbumArt(const std::string& filepath, const std::string& id, const audio::AlbumArt& art)
{
    // resize the album art if it is present
    if (!art.data.empty())
    {
        try
        {
            auto image = image::Factory::createFromData(art.data); // TODO: hint the proper image type from image.format
            image->resize(ALBUM_ART_CACHE_SIZE, ALBUM_ART_CACHE_SIZE, image::ResizeAlgorithm::Bilinear);
            m_jpgLoadStore->storeToFile(*image, fileops::combinePath(m_cacheDir, id + "_thumb.jpg"));
            return true;
        }
        catch (std::exception& e)
        {
            log::warn("Failed to scale image: %s", e.what());
        }
    }
    else
    {
        //no embedded album art found, see if we can find a cover.jpg, ... file
        return checkAlbumArt(fileops::getPathFromFilepath(filepath), id);
    }
    
    return false;
}

}
