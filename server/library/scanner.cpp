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
#include "utils/timeoperations.h"
#include "utils/log.h"

#include "md5.h"
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

static const int64_t g_rootId = 0;
static const int64_t g_musicId = 1;
static const int64_t g_albumsId = 2;
static const int64_t g_browseFileSystemId = 3;

Scanner::Scanner(const std::vector<std::string>& albumArtFilenames,  const std::string& dbPath, const std::string& cacheDir)
: m_libraryDb(dbPath)
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
    log::info("Starting library scan in: {}", libraryPath);

    m_initialScan = m_libraryDb.getObjectCount() == 0;
    m_scannedFiles = 0;
    
    if (m_initialScan)
    {
        createInitialLayout();
    }
    
    scan(libraryPath, g_browseFileSystemId);
    log::info("Library scan took {} seconds. Scanned {} files.", time(nullptr) - startTime, m_scannedFiles);
}

void Scanner::createInitialLayout()
{
    LibraryMetadata meta;

    // Root Item
    LibraryItem root;
    root.name = PACKAGE_NAME;
    root.objectId = g_rootId;
    root.parentId = -1;
    root.upnpClass = toString(upnp::Class::Container);
    meta.title = root.name;
    m_libraryDb.addItemWithId(root, meta);

    // Music
    LibraryItem music;
    music.name = "Music";
    music.objectId = g_musicId;
    music.parentId = g_rootId;
    music.upnpClass = toString(upnp::Class::Container);
    meta.title = music.name;
    m_libraryDb.addItemWithId(music, meta);

    // Music -> Albums
    LibraryItem albums;
    albums.name = "Albums";
    albums.objectId = g_albumsId;
    albums.parentId = music.objectId;
    albums.upnpClass = toString(upnp::Class::Container);
    meta.title = albums.name;
    m_libraryDb.addItemWithId(albums, meta);

    // Browse folders
    LibraryItem browse;
    browse.name = "Browse filesystem";
    browse.objectId = g_browseFileSystemId;
    browse.parentId = g_rootId;
    browse.upnpClass = toString(upnp::Class::StorageFolder);
    meta.title = browse.name;
    m_libraryDb.addItemWithId(browse, meta);
}

void Scanner::scan(const std::string& dir, int64_t parentId)
{
    std::vector<std::pair<std::vector<LibraryItem>, LibraryMetadata>> items;

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
            int64_t objectId;
            if (!m_libraryDb.itemExists(entry.path(), objectId))
            {
                LibraryItem item;
                item.name = fileops::getFileName(path);
                item.parentId = parentId;
                item.upnpClass = toString(upnp::Class::Container);

                LibraryMetadata meta;
                meta.title = item.name;
                meta.path = path;

                std::string hash;
                if (checkAlbumArt(meta.path, hash))
                {
                    meta.thumbnail = hash + "_thumb.jpg";
                }

                objectId = m_libraryDb.addItem(item, meta);
                log::debug("Add container: {} ({}) parent: {}", entry.path(), objectId, parentId);
            }

            scan(path, objectId);
        }
        else if (type == FileSystemEntryType::File)
        {
            try
            {
                auto path = entry.path();
                onFile(path, parentId, items);
            }
            catch (std::exception& e)
            {
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

void Scanner::onFile(const std::string& filepath, int64_t parentId, std::vector<std::pair<std::vector<LibraryItem>, LibraryMetadata>>& items)
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
    
    std::vector<LibraryItem> curItems;

    LibraryItem item;
    item.name = fileops::getFileName(filepath);
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

    LibraryMetadata meta;
    meta.path = filepath;
    meta.modifiedTime = info.modifyTime;
    meta.fileSize = info.sizeInBytes;
    meta.mimeType = toString(mime::typeFromFile(filepath));
    
    if (type == mime::Group::Audio)
    {
        try
        {
            audio::Metadata md(filepath, audio::Metadata::ReadAudioProperties::Yes);
            meta.artist         = md.getArtist();
            meta.title          = md.getTitle();
            meta.album          = md.getAlbum();
            meta.albumArtist    = md.getAlbumArtist();
            meta.genre          = md.getGenre();
            meta.date           = std::to_string(md.getYear());
            meta.trackNumber    = md.getTrackNr();
            meta.duration       = md.getDuration();
            meta.nrChannels     = md.getChannels();
            meta.sampleRate     = md.getSampleRate();
            meta.bitrate        = md.getBitRate();

            std::string hash;
            auto art = md.getAlbumArt();
            if (processAlbumArt(filepath, art, hash))
            {
                meta.thumbnail = hash + "_thumb.jpg";
            }
            
            // Add the album for this song if it is not already added
            if (!meta.album.empty())
            {
                try
                {
                    int64_t albumId;
                    if (!m_libraryDb.albumExists(meta.album, meta.albumArtist, albumId))
                    {
                        // first song we encounter of this album, add the album to the database
                        LibraryItem album;
                        album.parentId  = g_albumsId;
                        album.name      = meta.album;
                        album.upnpClass = "object.container.album.musicAlbum";
                        
                        LibraryMetadata albumMeta;
                        albumMeta.title     = meta.album;
                        albumMeta.artist    = meta.albumArtist;
                        albumMeta.date      = meta.date;
                        albumMeta.thumbnail = meta.thumbnail;
                        albumMeta.genre     = meta.genre;
                        albumMeta.dateAdded = timeops::getTime();
                        
                        log::debug("Add Album: {} - {}", albumMeta.artist, albumMeta.title);
                        albumId = m_libraryDb.addItem(album, albumMeta);
                        log::debug("Album id: {}", albumId);
                    }

                    // add song item as child of album
                    LibraryItem albumSongItem;
                    albumSongItem.name = item.name;
                    albumSongItem.parentId = albumId;
                    albumSongItem.upnpClass = item.upnpClass;
                    log::debug("Add album song: {} (parent: {})", albumSongItem.name, albumId);
                    curItems.push_back(std::move(albumSongItem));
                }
                catch (std::exception& e)
                {
                    log::warn("Failed to add album: {}", e.what());
                }
            }
        }
        catch (std::exception& e)
        {
            log::warn("Failed to add item: {}", e.what());
        }
    }
    
    log::debug("Add Item: {} (parent: {})", filepath, parentId);
    curItems.push_back(std::move(item));
    items.emplace_back(std::move(curItems), std::move(meta));
}

bool Scanner::checkAlbumArt(const std::string& directoryPath, std::string& hashString)
{
    for (auto& artName : m_albumArtFilenames)
    {
        try
        {
            auto possibleAlbumArt = fileops::combinePath(directoryPath, artName);
            if (fileops::pathExists(possibleAlbumArt))
            {
                auto data = fileops::readFile(possibleAlbumArt);
                hashString = createMd5String(data);

                auto image = image::Factory::createFromData(data);
                image->resize(ALBUM_ART_CACHE_SIZE, ALBUM_ART_CACHE_SIZE, image::ResizeAlgorithm::Bilinear);
                m_jpgLoadStore->storeToFile(*image, fileops::combinePath(m_cacheDir, hashString + "_thumb.jpg"));
                return true;
            }
        }
        catch (std::exception&) {}
    }

    return false;
}

bool Scanner::processAlbumArt(const std::string& filepath, const audio::AlbumArt& art, std::string& hashString)
{
    // resize the album art if it is present
    if (!art.data.empty())
    {
        hashString = createMd5String(art.data);

        if (fileops::pathExists(fileops::combinePath(m_cacheDir, hashString + "_thumb.jpg")))
        {
            // already cached an image with this hash
            return true;
        }

        try
        {
            auto image = image::Factory::createFromData(art.data); // TODO: hint the proper image type from image.format
            image->resize(ALBUM_ART_CACHE_SIZE, ALBUM_ART_CACHE_SIZE, image::ResizeAlgorithm::Bilinear);
            m_jpgLoadStore->storeToFile(*image, fileops::combinePath(m_cacheDir, hashString + "_thumb.jpg"));
            return true;
        }
        catch (std::exception& e)
        {
            log::warn("Failed to scale image: {}", e.what());
        }
    }
    else
    {
        //no embedded album art found, see if we can find a cover.jpg, ... file
        return checkAlbumArt(fileops::getPathFromFilepath(filepath), hashString);
    }
    
    return false;
}

}
