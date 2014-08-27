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

#include "musicdb.h"

#include <stdexcept>
#include <sstream>
#include <cassert>
#include <cstring>
#include <sqlite3.h>
#include <set>
#include <map>

#include "utils/fileoperations.h"
#include "utils/stringoperations.h"
#include "utils/numericoperations.h"
#include "utils/log.h"
#include "utils/trace.h"

#include <sqlpp11/sqlpp11.h>

#include "upnp/upnpitem.h"
#include "mimetypes.h"
#include "doozyscheme.h"

using namespace std;
using namespace utils;

//#define DEBUG_QUERIES

namespace doozy
{

namespace sql = sqlpp::sqlite3;

namespace
{

doozy::Objects objects;
doozy::Metadata metadata;

SQLPP_ALIAS_PROVIDER(numObjects);
SQLPP_ALIAS_PROVIDER(objectId);

template <typename T>
bool getIdFromResultIfExists(const T& result, int64_t& id)
{
    bool hasResults = !result.empty();
    if (hasResults)
    {
        id = result.front().objectId;
    }
    
    return hasResults;
}

sql::connection_config createConfig(const std::string& filename)
{
    sql::connection_config config;
    config.path_to_database = filename;
    config.flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    
#ifdef DEBUG_QUERIES
    config.debug = true;
#endif

    return config;
}

// Queries
auto objectCountQuery = [] () {
    return select(count(objects.Id).as(numObjects))
           .from(objects)
           .where(true);
};

auto childCountQuery = [] () {
    return select(count(objects.Id).as(numObjects))
           .from(objects)
           .where(objects.ParentId == parameter(objects.ParentId));
};

auto addItemQuery = [] () {
    return insert_into(objects).set(
        objects.ParentId    = parameter(objects.ParentId),
        objects.RefId       = parameter(objects.RefId),
        objects.Name        = parameter(objects.Name),
        objects.Class       = parameter(objects.Class),
        objects.MetaData    = parameter(objects.MetaData)
    );
};

auto addMetadataQuery = [] () {
    return insert_into(metadata).set(
        metadata.ModifiedTime   = parameter(metadata.ModifiedTime),
        metadata.FilePath       = parameter(metadata.FilePath),
        metadata.FileSize       = parameter(metadata.FileSize),
        metadata.Title          = parameter(metadata.Title),
        metadata.Artist         = parameter(metadata.Artist),
        metadata.Album          = parameter(metadata.Album),
        metadata.AlbumArtist    = parameter(metadata.AlbumArtist),
        metadata.Genre          = parameter(metadata.Genre),
        metadata.MimeType       = parameter(metadata.MimeType),
        metadata.Duration       = parameter(metadata.Duration),
        metadata.Channels       = parameter(metadata.Channels),
        metadata.BitRate        = parameter(metadata.BitRate),
        metadata.SampleRate     = parameter(metadata.SampleRate),
        metadata.Thumbnail      = parameter(metadata.Thumbnail)
    );
};

auto itemExistsQuery = [] () {
    return select(objects.Id.as(objectId))
           .from(metadata.inner_join(objects).on(objects.MetaData == metadata.Id))
           .where(metadata.FilePath == parameter(metadata.FilePath));
};

auto albumExistsQuery = [] () {
    return select(objects.Id.as(objectId))
           .from(metadata.inner_join(objects).on(objects.MetaData == metadata.Id))
           .where(objects.Class == "object.container.album.musicAlbum" and objects.Name == parameter(objects.Name) and metadata.Artist == parameter(metadata.Artist));
};

auto albumExistsQueryNoArtist = [] () {
    return select(objects.Id.as(objectId))
           .from(metadata.inner_join(objects).on(objects.MetaData == metadata.Id))
           .where(objects.Class == "object.container.album.musicAlbum" and objects.Name == parameter(objects.Name) and metadata.Artist.is_null());
};


auto itemStatusQuery = [] () {
    return select(metadata.ModifiedTime)
           .from(metadata)
           .where(metadata.FilePath == parameter(metadata.FilePath));
};

auto itemPathQuery = [] () {
    return select(metadata.FilePath)
           .from(objects.inner_join(metadata).on(objects.MetaData == metadata.Id))
           .where(objects.Id == parameter(objects.Id));
};

using ObjectCountQuery          = decltype(objectCountQuery());
using ChildCountQuery           = decltype(childCountQuery());
using AddItemQuery              = decltype(addItemQuery());
using AddMetadataQuery          = decltype(addMetadataQuery());
using ItemExistsQuery           = decltype(itemExistsQuery());
using ItemStatusQuery           = decltype(itemStatusQuery());
using ItemPathQuery             = decltype(itemPathQuery());
using AlbumExistsQuery          = decltype(albumExistsQuery());
using AlbumExistsQueryNoArtist  = decltype(albumExistsQueryNoArtist());

template <typename SelectType>
using PreparedStatement = decltype(((sql::connection*)nullptr)->prepare(*((SelectType*)nullptr)));

}

struct MusicDb::PreparedStatements
{
    PreparedStatement<ObjectCountQuery> objectCount;
    PreparedStatement<ChildCountQuery> childCount;
    PreparedStatement<AddItemQuery> addItem;
    PreparedStatement<AddMetadataQuery> addMetadata;
    PreparedStatement<ItemExistsQuery> itemExists;
    PreparedStatement<ItemStatusQuery> itemStatus;
    PreparedStatement<ItemPathQuery> itemPath;
    PreparedStatement<AlbumExistsQuery> albumExists;
    PreparedStatement<AlbumExistsQueryNoArtist> albumExistsNoArtist;
};

MusicDb::MusicDb(const string& dbFilepath)
: m_db(createConfig(dbFilepath))
, m_statements(new PreparedStatements())
{
    utils::trace("Create Music database");
    
    createInitialDatabase();
    prepareStatements();

    utils::trace("Music database loaded");
}

MusicDb::~MusicDb()
{
}

void MusicDb::prepareStatements()
{
    m_statements->objectCount           = m_db.prepare(objectCountQuery());
    m_statements->childCount            = m_db.prepare(childCountQuery());
    m_statements->addItem               = m_db.prepare(addItemQuery());
    m_statements->addMetadata           = m_db.prepare(addMetadataQuery());
    m_statements->itemExists            = m_db.prepare(itemExistsQuery());
    m_statements->itemStatus            = m_db.prepare(itemStatusQuery());
    m_statements->itemPath              = m_db.prepare(itemPathQuery());
    m_statements->albumExists           = m_db.prepare(albumExistsQuery());
    m_statements->albumExistsNoArtist   = m_db.prepare(albumExistsQueryNoArtist());
}

void MusicDb::setWebRoot(const std::string& webRoot)
{
    m_webRoot = webRoot;
}

uint64_t MusicDb::getObjectCount()
{
    return m_db.run(m_statements->objectCount).front().numObjects;
}

uint64_t MusicDb::getChildCount(int64_t id)
{
    m_statements->childCount.params.ParentId = id;
    return m_db.run(m_statements->childCount).front().numObjects;
}

void MusicDb::addItemWithId(const LibraryItem& item, const LibraryMetadata& meta)
{
    auto metaId = addMetadata(meta);

    m_db.run(insert_into(objects).set(
        objects.Id          = item.objectId,
        objects.ParentId    = item.parentId,
        objects.RefId       = sqlpp::tvin(item.refId),
        objects.Name        = item.name,
        objects.Class       = sqlpp::tvin(item.upnpClass),
        objects.MetaData    = metaId
    ));
}

int64_t MusicDb::addItem(const LibraryItem& item)
{
    m_statements->addItem.params.ParentId = sqlpp::tvin(item.parentId);
    m_statements->addItem.params.RefId    = sqlpp::tvin(item.refId);
    m_statements->addItem.params.Name     = item.name;
    m_statements->addItem.params.Class    = sqlpp::tvin(item.upnpClass);
    m_statements->addItem.params.MetaData = -1;
    return m_db.run(m_statements->addItem);
}

int64_t MusicDb::addItem(const LibraryItem& item, const LibraryMetadata& meta)
{
    return addItem(item, addMetadata(meta));
}

void MusicDb::addItems(const std::vector<std::pair<LibraryItem, LibraryMetadata>>& items)
{
    m_db.start_transaction();
    
    for (auto& item : items)
    {
        addItem(item.first, item.second);
    }
    
    m_db.commit_transaction();
}

void MusicDb::addItems(const std::vector<std::pair<std::vector<LibraryItem>, LibraryMetadata>>& items)
{
    m_db.start_transaction();
    
    for (auto& itemEntry : items)
    {
        auto metaId = addMetadata(itemEntry.second);
        for (auto& item : itemEntry.first)
        {
            addItem(item, metaId);
        }
    }
    
    m_db.commit_transaction();
}

int64_t MusicDb::addItem(const LibraryItem& item, int64_t metaId)
{
    m_statements->addItem.params.ParentId = item.parentId;
    m_statements->addItem.params.RefId    = sqlpp::tvin(item.refId);
    m_statements->addItem.params.Name     = item.name;
    m_statements->addItem.params.Class    = sqlpp::tvin(item.upnpClass);
    m_statements->addItem.params.MetaData = metaId;
    return m_db.run(m_statements->addItem);
}

int64_t MusicDb::addMetadata(const LibraryMetadata& meta)
{
    m_statements->addMetadata.params.ModifiedTime  = static_cast<int64_t>(meta.modifiedTime);
    m_statements->addMetadata.params.FilePath      = sqlpp::tvin(meta.path);
    m_statements->addMetadata.params.FileSize      = static_cast<int64_t>(meta.fileSize);
    m_statements->addMetadata.params.Title         = sqlpp::tvin(meta.title);
    m_statements->addMetadata.params.Artist        = sqlpp::tvin(meta.artist);
    m_statements->addMetadata.params.Album         = sqlpp::tvin(meta.album);
    m_statements->addMetadata.params.AlbumArtist   = sqlpp::tvin(meta.albumArtist);
    m_statements->addMetadata.params.Genre         = sqlpp::tvin(meta.genre);
    m_statements->addMetadata.params.MimeType      = sqlpp::tvin(meta.mimeType);
    m_statements->addMetadata.params.Duration      = meta.duration;
    m_statements->addMetadata.params.Channels      = meta.nrChannels;
    m_statements->addMetadata.params.BitRate       = meta.bitrate;
    m_statements->addMetadata.params.SampleRate    = meta.sampleRate;
    m_statements->addMetadata.params.Thumbnail     = sqlpp::tvin(meta.thumbnail);
    
    return m_db.run(m_statements->addMetadata);
}

void MusicDb::updateItem(const LibraryItem& item, const LibraryMetadata& meta)
{
    m_db.run(update(objects).set(
        objects.ParentId   = item.parentId,
        objects.RefId      = sqlpp::tvin(item.refId),
        objects.Name       = item.name,
        objects.Class      = sqlpp::tvin(item.upnpClass)
    ).where(objects.Id == item.objectId));
    
    m_db.run(update(metadata).set(
        metadata.ModifiedTime  = static_cast<int64_t>(meta.modifiedTime),
        metadata.FilePath      = sqlpp::tvin(meta.path),
        metadata.FileSize      = static_cast<int64_t>(meta.fileSize),
        metadata.Title         = sqlpp::tvin(meta.title),
        metadata.Artist        = sqlpp::tvin(meta.artist),
        metadata.Genre         = sqlpp::tvin(meta.genre),
        metadata.MimeType      = sqlpp::tvin(meta.mimeType),
        metadata.Duration      = meta.duration,
        metadata.Channels      = meta.nrChannels,
        metadata.BitRate       = meta.bitrate,
        metadata.SampleRate    = meta.sampleRate,
        metadata.Thumbnail     = sqlpp::tvin(meta.thumbnail)
    ).where(true));
}

bool MusicDb::itemExists(const string& filepath, int64_t& objectId)
{
    m_statements->itemExists.params.FilePath = filepath;
    auto result = m_db.run(m_statements->itemExists);
    
    if (result.empty())
    {
        return false;
    }
    
    objectId = result.front().objectId;
    return true;
}

bool MusicDb::albumExists(const std::string& title, const std::string& artist, int64_t& objectId)
{
    if (artist.empty())
    {
        m_statements->albumExistsNoArtist.params.Name = title;
        auto result = m_db.run(m_statements->albumExistsNoArtist);
        if (result.empty())
        {
            return false;
        }

        objectId = result.front().objectId;
    }
    else
    {
        m_statements->albumExists.params.Name = title;
        m_statements->albumExists.params.Artist = artist;
        auto result = m_db.run(m_statements->albumExists);
        if (result.empty())
        {
            return false;
        }
        
        objectId = result.front().objectId;
    }

    return true;
}

ItemStatus MusicDb::getItemStatus(const std::string& filepath, uint64_t modifiedTime)
{
    m_statements->itemStatus.params.FilePath = filepath;
    auto result = m_db.run(m_statements->itemStatus);
    
    if (result.empty())
    {
        return ItemStatus::DoesntExist;
    }
    
    return static_cast<uint64_t>(result.front().ModifiedTime) < modifiedTime ? ItemStatus::NeedsUpdate : ItemStatus::UpToDate;
}

std::string MusicDb::getItemPath(int64_t objectId)
{
    m_statements->itemPath.params.Id = objectId;
    auto result = m_db.run(m_statements->itemPath);
    
    if (result.empty())
    {
        throw std::runtime_error(stringops::format("No item in database with id: %d", objectId));
    }

    return result.front().FilePath;
}

template <typename T>
upnp::ItemPtr MusicDb::parseItem(const T& row)
{
    assert(!m_webRoot.empty());

    auto item = std::make_shared<upnp::Item>();
    item->setObjectId(std::to_string(row.objectId));
    item->setTitle(row.Name);
    item->setParentId(std::to_string(row.ParentId));
    item->setClass(row.Class);
    item->addMetaData(upnp::Property::Artist, row.Artist);
    
    if (!row.RefId.is_null())
    {
        item->setRefId(std::to_string(row.RefId));
    }    

    if (!row.Title.is_null())
    {
        item->setTitle(row.Title);
    }

    item->addMetaData(upnp::Property::Genre, row.Genre);

    if (!item->isContainer())
    {
        // add the resource urls
        upnp::Resource res;
        auto mimeType = row.MimeType;
        res.setProtocolInfo(upnp::ProtocolInfo(stringops::format("http-get:*:%s:*", mimeType)));
        res.setUrl(stringops::format("%sMedia/%s.%s", m_webRoot, item->getObjectId(), mime::extensionFromType(mime::typeFromString(mimeType))));
        res.setSize(row.FileSize);
        res.setDuration(static_cast<uint32_t>(row.Duration));
        res.setNrAudioChannels(static_cast<uint32_t>(row.Channels));
        res.setBitRate(static_cast<uint32_t>(row.BitRate));
        res.setSampleRate(static_cast<uint32_t>(row.SampleRate));

        item->addResource(res);
    }
    else
    {
        item->addMetaData(upnp::Property::StorageUsed, "-1");
    }
    
    auto thumbnail = row.Thumbnail;
    if (!thumbnail.is_null())
    {
        item->addMetaData(upnp::Property::AlbumArt, fileops::combinePath(m_webRoot, thumbnail));
    }

    return item;
}

upnp::ItemPtr MusicDb::getItem(int64_t id)
{
    const auto& result = m_db.run(
        select(objects.Id.as(objectId), objects.Name, objects.ParentId, objects.RefId, objects.Class, all_of(metadata))
        .from(objects.left_outer_join(metadata).on(objects.MetaData == metadata.Id))
        .where(objects.Id == id)
    );

    if (result.empty())
    {
        throw std::runtime_error(stringops::format("No track found in db with id: %d", + id));
    }

    return parseItem(result.front());
}

std::vector<upnp::ItemPtr> MusicDb::getItems(int64_t parentId, uint32_t offset, uint32_t count)
{
    std::vector<upnp::ItemPtr> items;

    for (const auto& row : m_db.run(
        select(objects.Id.as(objectId), objects.Name, objects.ParentId, objects.RefId, objects.Class, all_of(metadata))
        .from(objects.left_outer_join(metadata).on(objects.MetaData == metadata.Id))
        .where(objects.ParentId == parentId)
        .limit(count == 0 ? -1 : count)
        .offset(offset)
    ))
    {
        items.emplace_back(parseItem(row));
    }

    return items;
}

void MusicDb::removeItem(int64_t id)
{
    m_db.run(
        remove_from(objects)
        .where(objects.Id == id)
    );
}

void MusicDb::removeMetaData(int64_t id)
{
    m_db.run(
        remove_from(metadata)
        .where(metadata.Id == static_cast<int64_t>(id))
    );
}

void MusicDb::removeNonExistingFiles()
{
    std::vector<std::pair<int64_t, int64_t>> files;

    for (const auto& row : m_db.run(select(objects.Id.as(objectId), objects.MetaData, metadata.FilePath)
                                     .from(objects.left_outer_join(metadata).on(objects.MetaData == metadata.Id))
                                     .where(true)))
    {
        std::string path = row.FilePath;
        if (!path.empty() && !fileops::pathExists(path))
        {
            files.emplace_back(row.objectId, row.MetaData);
        }
    }
    
    for (auto& iter : files)
    {
        log::debug("Removed deleted file from database: %d", iter.first);
        removeItem(iter.first);
        removeMetaData(iter.second);
    }
}

//class SearchTrackData
//{
//public:
//    SearchTrackData(utils::ISubscriber<const Track&>& trackSubscriber, set<uint32_t>& foundIds)
//    : subscriber(trackSubscriber)
//    , ids(foundIds)
//    {
//    }
//
//    utils::ISubscriber<const Track&>& subscriber;
//    set<uint32_t>&      ids;
//};
//
//void MusicDb::searchLibrary(const std::string& search, utils::ISubscriber<const Track&>& trackSubscriber, utils::ISubscriber<const Album&>& albumSubscriber)
//{
//    std::lock_guard<std::recursive_mutex> lock(m_dbMutex);
//
//    set<uint32_t> albumIds;
//    sqlite3_stmt* pStmt = createStatement(
//        "SELECT Id "
//        "FROM albums "
//        "WHERE albums.AlbumArtist LIKE (SELECT '%' || ?1 || '%') "
//        "OR albums.Name LIKE (SELECT '%' || ?1 || '%');");
//    bindValue(pStmt, search, 1);
//    performQuery(pStmt, getAllAlbumIdsCb, &albumIds);
//
//    SearchTrackData data(trackSubscriber, albumIds);
//    pStmt = createStatement(
//        "SELECT tracks.Id, tracks.AlbumId, tracks.Title, tracks.Composer, tracks.Filepath, tracks.Year, tracks.TrackNr, tracks.DiscNr, tracks.Duration, tracks.BitRate, tracks.SampleRate, tracks.Channels, tracks.FileSize, tracks.ModifiedTime, artists.Name, albums.Name, albums.AlbumArtist, genres.Name "
//        "FROM tracks "
//        "LEFT OUTER JOIN albums ON tracks.AlbumId = albums.Id "
//        "LEFT OUTER JOIN artists ON tracks.ArtistId = artists.Id "
//        "LEFT OUTER JOIN genres ON tracks.GenreId = genres.Id "
//        "WHERE artists.Name LIKE (SELECT '%' || ?1 || '%')"
//        "OR tracks.title LIKE (SELECT '%' || ?1 || '%');");
//
//
//    bindValue(pStmt, search, 1);
//    performQuery(pStmt, searchTracksCb, &data);
//
//    vector<Album> albums;
//    pStmt = createStatement(
//        "SELECT albums.Id, albums.Name, albums.AlbumArtist, albums.Year, albums.Duration, albums.DateAdded, genres.Name "
//        "FROM albums "
//    	"LEFT OUTER JOIN genres ON albums.GenreId = genres.Id "
//        "WHERE albums.Id=?;");
//
//    for (set<uint32_t>::iterator iter = data.ids.begin(); iter != data.ids.end(); ++iter)
//    {
//        bindValue(pStmt, *iter, 1);
//        performQuery(pStmt, getAlbumListCb, &albums, false);
//
//        if (sqlite3_reset(pStmt) != SQLITE_OK)
//        {
//            sqlite3_finalize(pStmt);
//            throw runtime_error(string("Failed to reset statement: ") + sqlite3_errmsg(m_pDb));
//        }
//        if (sqlite3_clear_bindings(pStmt) != SQLITE_OK)
//        {
//            sqlite3_finalize(pStmt);
//            throw runtime_error(string("Failed to clear bindings: ") + sqlite3_errmsg(m_pDb));
//        }
//    }
//    sqlite3_finalize(pStmt);
//
//    for (size_t i = 0; i < albums.size(); ++i)
//    {
//        albumSubscriber.onItem(albums[i]);
//    }
//}

void MusicDb::clearDatabase()
{
    m_db.execute("DROP INDEX IF EXISTS metadata.pathIndex;");
    m_db.execute("DROP TABLE IF EXISTS objects;");
    m_db.execute("DROP TABLE IF EXISTS metadata;");

    createInitialDatabase();
}

void MusicDb::createInitialDatabase()
{
    m_db.execute("CREATE TABLE IF NOT EXISTS objects("
        "Id INTEGER PRIMARY KEY,"
        "ParentId INTEGER NOT NULL,"
        "RefId INTEGER,"
        "Name TEXT NOT NULL,"
        "Class TEXT,"
        "MetaData INTEGER,"
        "FOREIGN KEY (MetaData) REFERENCES metadata(id));");

    m_db.execute("CREATE TABLE IF NOT EXISTS metadata("
        "Id INTEGER PRIMARY KEY,"
        "Album TEXT,"
        "Artist TEXT,"
        "Title TEXT,"
        "AlbumArtist TEXT,"
        "Genre TEXT,"
        "Composer TEXT,"
        "Year INTEGER,"
        "TrackNr INTEGER,"
        "DiscNr INTEGER,"
        "AlbumOrder INTEGER,"
        "Duration INTEGER,"
        "MimeType TEXT,"
        "BitRate INTEGER,"
        "SampleRate INTEGER,"
        "Channels INTEGER,"
        "FileSize INTEGER,"
        "DateAdded INTEGER,"
        "ModifiedTime INTEGER,"
        "Thumbnail TEXT,"
        "FilePath TEXT);");

    m_db.execute("CREATE INDEX IF NOT EXISTS pathIndex ON metadata (FilePath);");
}

}
