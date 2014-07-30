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

#include "subscribers.h"
#include "utils/fileoperations.h"
#include "utils/stringoperations.h"
#include "utils/numericoperations.h"
#include "utils/log.h"
#include "utils/trace.h"

#include "upnp/upnpitem.h"
#include "mimetypes.h"

using namespace std;
using namespace utils;

#define BUSY_RETRIES 50

namespace doozy
{

namespace
{

std::string getStringFromColumn(sqlite3_stmt* pStmt, int column)
{
    const char* pString = reinterpret_cast<const char*>(sqlite3_column_text(pStmt, column));
    if (pString != nullptr)
    {
        return pString;
    }

    return std::string();
}

std::string getStringCb(sqlite3_stmt* pStmt)
{
    assert(sqlite3_column_count(pStmt) == 1);
    assert(sqlite3_column_text(pStmt, 0));

    return getStringFromColumn(pStmt, 0);
}

uint64_t getCountCb(sqlite3_stmt* pStmt)
{
    assert(sqlite3_column_count(pStmt) == 1);
    return sqlite3_column_int64(pStmt, 0);
}

}

MusicDb::MusicDb(const string& dbFilepath)
: m_pDb(nullptr)
{
    utils::trace("Create Music database");

    if (sqlite3_open(dbFilepath.c_str(), &m_pDb) != SQLITE_OK)
    {
        throw runtime_error("Failed to open database: " + dbFilepath);
    }

    if (sqlite3_busy_handler(m_pDb, MusicDb::busyCb, nullptr) != SQLITE_OK)
    {
        throw runtime_error("Failed to set busy handler");
    }
    
    m_pBeginStatement = createStatement("BEGIN");
    m_pCommitStatement = createStatement("COMMIT");

    createInitialDatabase();

    utils::trace("Music database loaded");
}

MusicDb::~MusicDb()
{
    sqlite3_finalize(m_pBeginStatement);
    sqlite3_finalize(m_pCommitStatement);

    if (sqlite3_close(m_pDb) != SQLITE_OK)
    {
        log::error("Failed to close database: " + std::string(sqlite3_errmsg(m_pDb)));
    }
}

void MusicDb::setWebRoot(const std::string& webRoot)
{
    m_webRoot = webRoot;
}

uint64_t MusicDb::getObjectCount()
{
    std::lock_guard<std::recursive_mutex> lock(m_dbMutex);

    auto stmt = createStatement("SELECT COUNT(Id) FROM objects");

    uint64_t count;
    performQuery(stmt, true, [&] () {
        count = getCountCb(stmt);
    });

    return count;
}

uint64_t MusicDb::getChildCount(const std::string& id)
{
    std::lock_guard<std::recursive_mutex> lock(m_dbMutex);
    auto stmt = createStatement(
        "SELECT Id FROM objects "
        "WHERE objects.ParentId=?");

    bindValue(stmt, id, 1);
    return performQuery(stmt);
}

uint64_t MusicDb::getUniqueIdInContainer(const std::string& containerId)
{
    std::lock_guard<std::recursive_mutex> lock(m_dbMutex);
    auto stmt = createStatement(
        "SELECT COUNT(Id) FROM objects "
        "WHERE objects.ParentId=?"
    );

    bindValue(stmt, containerId, 1);

    uint64_t id;
    performQuery(stmt, true, [&] () {
        id = getCountCb(stmt);
    });

    return id;
}

void MusicDb::addItem(const LibraryItem& item)
{
    std::lock_guard<std::recursive_mutex> lock(m_dbMutex);
    auto metaId = addMetadata(item);
    
    sqlite3_stmt* pStmt = createStatement(
        "INSERT INTO objects "
        "(Id, ObjectId, ParentId, RefId, Name, Class, MetaData) "
        "VALUES (NULL, ?, ?, ?, ?, ?, ?)");

    // create copies of temporary values, the query keeps a pointer to it
    bindValue(pStmt, item.objectId, 1);
    bindValue(pStmt, item.parentId, 2);
    bindValue(pStmt, item.refId, 3);
    bindValue(pStmt, item.name, 4);
    bindValue(pStmt, item.upnpClass, 5);
    bindValue(pStmt, metaId, 6);
    performQuery(pStmt);
}

void MusicDb::addItems(const std::vector<LibraryItem>& items)
{
    std::lock_guard<std::recursive_mutex> lock(m_dbMutex);
    
    sqlite3_stmt* pMetaStmt = createStatement(
        "INSERT INTO metadata "
        "(Id, ModifiedTime, FilePath, FileSize, Title, Artist, Genre, MimeType, Duration, Channels, BitRate, SampleRate, Thumbnail) "
        "VALUES (NULL, ?,   ?,        ?,        ?,     ?,      ?,     ?,        ?,        ?,        ?,       ?,          ?)");
    
    sqlite3_stmt* pStmt = createStatement(
        "INSERT INTO objects "
        "(Id, ObjectId, ParentId, RefId, Name, Class, MetaData) "
        "VALUES (NULL, ?, ?, ?, ?, ?, last_insert_rowid())");

    performQuery(m_pBeginStatement, false);
    for (auto& item : items)
    {
        bindValue(pMetaStmt, item.modifiedTime, 1);
        bindValue(pMetaStmt, item.path, 2);
        bindValue(pMetaStmt, item.fileSize, 3);
        bindValue(pMetaStmt, item.title, 4);
        bindValue(pMetaStmt, item.artist, 5);
        bindValue(pMetaStmt, item.genre, 6);
        bindValue(pMetaStmt, item.mimeType, 7);
        bindValue(pMetaStmt, item.duration, 8);
        bindValue(pMetaStmt, item.nrChannels, 9);
        bindValue(pMetaStmt, item.bitrate, 10);
        bindValue(pMetaStmt, item.sampleRate, 11);
        bindValue(pMetaStmt, item.thumbnail, 12);
        performQuery(pMetaStmt, false);
        
        bindValue(pStmt, item.objectId, 1);
        bindValue(pStmt, item.parentId, 2);
        bindValue(pStmt, item.refId, 3);
        bindValue(pStmt, item.name, 4);
        bindValue(pStmt, item.upnpClass, 5);
        performQuery(pStmt, false);
    }
    
    performQuery(m_pCommitStatement, false);
    
    sqlite3_finalize(pStmt);
    sqlite3_finalize(pMetaStmt);
}

int64_t MusicDb::addMetadata(const LibraryItem& item)
{
    sqlite3_stmt* pStmt = createStatement(
        "INSERT INTO metadata "
        "(Id, ModifiedTime, FilePath, FileSize, Title, Artist, Genre, MimeType, Duration, Channels, BitRate, SampleRate, Thumbnail) "
        "VALUES (NULL, ?,   ?,        ?,        ?,     ?,      ?,     ?,        ?,        ?,        ?,       ?,          ?)"
    );
    
    bindValue(pStmt, item.modifiedTime, 1);
    bindValue(pStmt, item.path, 2);
    bindValue(pStmt, item.fileSize, 3);
    bindValue(pStmt, item.title, 4);
    bindValue(pStmt, item.artist, 5);
    bindValue(pStmt, item.genre, 6);
    bindValue(pStmt, item.mimeType, 7);
    bindValue(pStmt, item.duration, 8);
    bindValue(pStmt, item.nrChannels, 9);
    bindValue(pStmt, item.bitrate, 10);
    bindValue(pStmt, item.sampleRate, 11);
    bindValue(pStmt, item.thumbnail, 12);
    performQuery(pStmt);
    
    return sqlite3_last_insert_rowid(m_pDb);
}

void MusicDb::updateItem(const LibraryItem& item)
{
    std::lock_guard<std::recursive_mutex> lock(m_dbMutex);
    sqlite3_stmt* pStmt = createStatement(
        "UPDATE objects "
        "SET ParentId=?, RefId=?, Name=?, Class=? "
        "WHERE ObjectId=?"
    );

    bindValue(pStmt, item.parentId, 1);
    bindValue(pStmt, item.refId, 2);
    bindValue(pStmt, item.name, 3);
    bindValue(pStmt, item.upnpClass, 4);
    bindValue(pStmt, item.objectId, 5);
    performQuery(pStmt);
    
    pStmt = createStatement(
        "UPDATE metadata "
        "SET ModifiedTime=?, Title=?, Artist=?"
        "WHERE FilePath=?"
    );

    bindValue(pStmt, item.modifiedTime, 1);
    bindValue(pStmt, item.title, 2);
    bindValue(pStmt, item.artist, 3);
    bindValue(pStmt, item.path, 4);
    
    performQuery(pStmt);
}

bool MusicDb::itemExists(const string& filepath, string& objectId)
{
    std::lock_guard<std::recursive_mutex> lock(m_dbMutex);
    sqlite3_stmt* pStmt = createStatement(
        "SELECT objects.ObjectId "
        "FROM metadata "
        "LEFT OUTER JOIN objects ON objects.MetaData = metadata.Id "
        "WHERE metadata.FilePath=?"
    );
    
    if (sqlite3_bind_text(pStmt, 1, filepath.c_str(), static_cast<int>(filepath.size()), SQLITE_STATIC) != SQLITE_OK)
    {
        throw runtime_error(string("Failed to bind value: ") + sqlite3_errmsg(m_pDb));
    }

    auto numObjects = performQuery(pStmt, true, [&] () {
        objectId = getStringCb(pStmt);
    });
    assert(numObjects <= 1);
    return numObjects == 1;
}

ItemStatus MusicDb::getItemStatus(const std::string& filepath, uint64_t modifiedTime)
{
    std::lock_guard<std::recursive_mutex> lock(m_dbMutex);
    auto stmt = createStatement("SELECT ModifiedTime FROM metadata WHERE metadata.FilePath = ?");
    bindValue(stmt, filepath, 1);

    uint64_t dbModifiedTime;
    auto numTracks = performQuery(stmt, true, [&] () {
        dbModifiedTime = sqlite3_column_int64(stmt, 0);
    });

    assert(numTracks <= 1);
    if (numTracks == 0)
    {
        return ItemStatus::DoesntExist;
    }
    else if (dbModifiedTime < modifiedTime)
    {
        return ItemStatus::NeedsUpdate;
    }
    else
    {
        return ItemStatus::UpToDate;
    }
}

upnp::ItemPtr MusicDb::getItemCb(sqlite3_stmt* stmt)
{
    assert(sqlite3_column_count(stmt) == 15);
    assert(!m_webRoot.empty());

    auto item = std::make_shared<upnp::Item>();
    item->setObjectId(getStringFromColumn(stmt, 0));
    item->setTitle(getStringFromColumn(stmt, 1));
    item->setParentId(getStringFromColumn(stmt, 2));
    item->setRefId(getStringFromColumn(stmt, 3));
    item->setClass(getStringFromColumn(stmt, 4));
    item->addMetaData(upnp::Property::Artist, getStringFromColumn(stmt, 5));

    auto title = getStringFromColumn(stmt, 6);
    if (!title.empty())
    {
        item->setTitle(getStringFromColumn(stmt, 6));
    }

    item->addMetaData(upnp::Property::Genre, getStringFromColumn(stmt, 7));

    if (!item->isContainer())
    {
        // add the resource urls
        upnp::Resource res;
        auto mimeType = getStringFromColumn(stmt, 8);
        res.setProtocolInfo(upnp::ProtocolInfo(stringops::format("http-get:*:%s:*", mimeType)));
        res.setUrl(stringops::format("%sMedia/%s.%s", m_webRoot, item->getObjectId(), mime::extensionFromType(mime::typeFromString(mimeType))));
        res.setSize(sqlite3_column_int64(stmt, 9));
        res.setDuration(sqlite3_column_int(stmt, 10));
        res.setNrAudioChannels(sqlite3_column_int(stmt, 11));
        res.setBitRate(sqlite3_column_int(stmt, 12));
        res.setSampleRate(sqlite3_column_int(stmt, 13));

        item->addResource(res);
    }
    else
    {
        item->addMetaData(upnp::Property::StorageUsed, "-1");
    }
    
    auto thumbnail = getStringFromColumn(stmt, 14);
    if (!thumbnail.empty())
    {
        item->addMetaData(upnp::Property::AlbumArt, fileops::combinePath(m_webRoot, thumbnail));
    }

    return item;
}

upnp::ItemPtr MusicDb::getItem(const std::string& id)
{
    std::lock_guard<std::recursive_mutex> lock(m_dbMutex);
    auto stmt = createStatement(
        "SELECT o.ObjectId, o.Name, o.ParentId, o.RefId, o.Class, "
        "m.Artist, m.Title, m.Genre, m.MimeType, m.FileSize, m.Duration, m.Channels, m.BitRate, m.SampleRate, m.Thumbnail "
        "FROM objects AS o "
        "LEFT OUTER JOIN metadata AS m ON o.MetaData = m.Id "
        "WHERE o.ObjectId = ? ");

    bindValue(stmt, id, 1);

    upnp::ItemPtr item;
    auto count = performQuery(stmt, true, [&] () {
        item = getItemCb(stmt);
    });

    if (count == 0 || item->getObjectId().empty())
    {
        throw std::runtime_error("No track found in db with id: " + id);
    }

    return item;
}

std::vector<upnp::ItemPtr> MusicDb::getItems(const std::string& parentId, uint32_t offset, uint32_t count)
{
    std::lock_guard<std::recursive_mutex> lock(m_dbMutex);
    auto stmt = createStatement(
        "SELECT o.ObjectId, o.Name, o.ParentId, o.RefId, o.Class, "
        "m.Artist, m.Title, m.Genre, m.MimeType, m.FileSize, m.Duration, m.Channels, m.BitRate, m.SampleRate, m.Thumbnail "
        "FROM objects AS o "
        "LEFT OUTER JOIN metadata AS m ON o.MetaData = m.Id "
        "WHERE o.ParentId = ? LIMIT ? OFFSET ?");

    bindValue(stmt, parentId, 1);
    bindValue(stmt, count == 0 ? -1 : count, 2);
    bindValue(stmt, offset, 3);

    std::vector<upnp::ItemPtr> items;
    performQuery(stmt, true, [&] () {
        items.push_back(getItemCb(stmt));
    });
    return items;
}

std::string MusicDb::getItemPath(const std::string& id)
{
    std::lock_guard<std::recursive_mutex> lock(m_dbMutex);
    auto stmt = createStatement(
        "SELECT metadata.FilePath "
        "FROM objects "
        "LEFT OUTER JOIN metadata ON objects.MetaData = metadata.Id "
        "WHERE objects.ObjectId = ?");

    bindValue(stmt, id, 1);

    std::string path;
    auto count = performQuery(stmt, true, [&] () {
        path = getStringCb(stmt);
    });

    if (0 == count)
    {
        throw std::runtime_error("No item in database with id: " + id);
    }
    
    return path;
}

void MusicDb::removeItem(const std::string& id)
{
    std::lock_guard<std::recursive_mutex> lock(m_dbMutex);
    sqlite3_stmt* pStmt = createStatement("DELETE from objects WHERE Id = ?");
    bindValue(pStmt, id, 1);
    performQuery(pStmt);
}

void MusicDb::removeMetaData(const std::string& id)
{
    std::lock_guard<std::recursive_mutex> lock(m_dbMutex);
    sqlite3_stmt* pStmt = createStatement("DELETE from metadata WHERE Id = ?");
    bindValue(pStmt, id, 1);
    performQuery(pStmt);
}


void MusicDb::removeNonExistingFiles()
{
    std::lock_guard<std::recursive_mutex> lock(m_dbMutex);
    auto stmt = createStatement(
        "SELECT objects.Id, objects.MetaData, metadata.FilePath "
        "FROM objects "
        "LEFT OUTER JOIN metadata ON objects.MetaData = metadata.Id"
    );

    std::vector<std::pair<uint32_t, uint32_t>> files;
    performQuery(stmt, true, [&] () {
        string path = getStringFromColumn(stmt, 2);
        if (!path.empty() && !fileops::pathExists(path))
        {
            uint32_t objectsId = sqlite3_column_int(stmt, 0);
            uint32_t metaDataId = sqlite3_column_int(stmt, 1);
            files.push_back(std::make_pair(objectsId, metaDataId));
        }
    });

    for (size_t i = 0; i < files.size(); ++i)
    {
        log::debug("Removed deleted file from database: %d", files[i].first);
        removeItem(numericops::toString(files[i].first));
        removeMetaData(numericops::toString(files[i].second));
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
    std::lock_guard<std::recursive_mutex> lock(m_dbMutex);
    performQuery(createStatement("DROP INDEX IF EXISTS metadata.pathIndex;"));
    performQuery(createStatement("DROP TABLE IF EXISTS objects;"));
    performQuery(createStatement("DROP TABLE IF EXISTS metadata;"));

    createInitialDatabase();
}

void MusicDb::createInitialDatabase()
{
    std::lock_guard<std::recursive_mutex> lock(m_dbMutex);
    performQuery(createStatement("CREATE TABLE IF NOT EXISTS objects("
        "Id INTEGER PRIMARY KEY,"
        "ObjectId TEXT UNIQUE,"
        "ParentId TEXT,"
        "RefId TEXT,"
        "Name TEXT NOT NULL,"
        "Class TEXT,"
        "MetaData INTEGER,"
        "FOREIGN KEY (MetaData) REFERENCES metadata(id));"));

    performQuery(createStatement("CREATE TABLE IF NOT EXISTS metadata("
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
        "FilePath TEXT UNIQUE);"));

    performQuery(createStatement("CREATE UNIQUE INDEX IF NOT EXISTS pathIndex ON metadata (FilePath);"));
}

uint64_t MusicDb::performQuery(sqlite3_stmt* pStmt, bool finalize, std::function<void()> cb)
{
    uint64_t rowCount = 0;

    int32_t rc;
    while ((rc = sqlite3_step(pStmt)) != SQLITE_DONE)
    {
        switch(rc)
        {
        case SQLITE_BUSY:
            sqlite3_finalize(pStmt);
            throw runtime_error("Failed to execute statement: SQL is busy");
        case SQLITE_ERROR:
            sqlite3_finalize(pStmt);
            throw runtime_error(string("Failed to execute statement: ") + sqlite3_errmsg(m_pDb));
        case SQLITE_CONSTRAINT:
            sqlite3_finalize(pStmt);
            throw runtime_error(string("Sqlite constraint violated: ") + sqlite3_errmsg(m_pDb));
        case SQLITE_ROW:
            if (cb) cb();
            ++rowCount;
            break;
        default:
            sqlite3_finalize(pStmt);
            throw runtime_error("FIXME: unhandled return value of sql statement: " + numericops::toString(rc));
        }
    }

    if (finalize)
    {
        sqlite3_finalize(pStmt);
    }
    else
    {
        sqlite3_reset(pStmt);
    }
    
    return rowCount;
}

sqlite3_stmt* MusicDb::createStatement(const char* query)
{
    sqlite3_stmt* pStmt;

    if (sqlite3_prepare_v2(m_pDb, query, -1, &pStmt, 0) != SQLITE_OK)
    {
        throw runtime_error(string("Failed to prepare sql statement (") + sqlite3_errmsg(m_pDb) + "): " + query);
    }

    return pStmt;
}

void MusicDb::bindValue(sqlite3_stmt* pStmt, const string& value, int32_t index, bool copy)
{
    if (value.empty())
    {
        if (sqlite3_bind_null(pStmt, index) != SQLITE_OK)
        {
            throw runtime_error(string("Failed to bind string value as NULL: ") + sqlite3_errmsg(m_pDb));
        }
    }
    else if (sqlite3_bind_text(pStmt, index, value.c_str(), static_cast<int>(value.size()), copy ? SQLITE_TRANSIENT : SQLITE_STATIC) != SQLITE_OK)
    {
        throw runtime_error(string("Failed to bind string value: ") + sqlite3_errmsg(m_pDb));
    }
}

void MusicDb::bindValue(sqlite3_stmt* pStmt, uint32_t value, int32_t index)
{
    if (sqlite3_bind_int(pStmt, index, value) != SQLITE_OK)
    {
        throw runtime_error(string("Failed to bind int value: ") + sqlite3_errmsg(m_pDb));
    }
}

void MusicDb::bindValue(sqlite3_stmt* pStmt, int64_t value, int32_t index)
{
    if (sqlite3_bind_int64(pStmt, index, value) != SQLITE_OK)
    {
        throw runtime_error(string("Failed to bind int64 value: ") + sqlite3_errmsg(m_pDb));
    }
}

void MusicDb::bindValue(sqlite3_stmt* pStmt, uint64_t value, int32_t index)
{
    bindValue(pStmt, static_cast<int64_t>(value), index);
}

void MusicDb::bindValue(sqlite3_stmt* pStmt, const void* pData, size_t dataSize, int32_t index)
{
    if (pData == nullptr)
    {
        if (sqlite3_bind_null(pStmt, index) != SQLITE_OK)
        {
            throw runtime_error(string("Failed to bind blob value as NULL: ") + sqlite3_errmsg(m_pDb));
        }
    }
    else if (sqlite3_bind_blob(pStmt, index, pData, static_cast<int>(dataSize), SQLITE_TRANSIENT) != SQLITE_OK)
    {
        throw runtime_error(string("Failed to bind int value: ") + sqlite3_errmsg(m_pDb));
    }
}

std::string MusicDb::getIdFromTableAsString(const string& table, const string& name)
{
    std::stringstream query;
    query << "SELECT Id FROM " << table << " WHERE Name = ?;";
    auto stmt = createStatement(("SELECT Id FROM " + table + " WHERE Name = ?;").c_str());
    bindValue(stmt, name, 1);

    std::string id;
    performQuery(stmt, true, [&] () {
        id = getStringCb(stmt);
    });

    return id;
}

uint64_t MusicDb::getIdFromTable(const string& table, const string& name)
{
    stringstream query;
    query << "SELECT Id FROM " << table << " WHERE ObjectId = ?;";
    auto stmt = createStatement(query.str().c_str());
    bindValue(stmt, name, 1);

    uint64_t id = 0;
    performQuery(stmt, true, [&] () {
        id = sqlite3_column_int64(stmt, 0);
    });

    return id;
}

//void MusicDb::getTracksCb(sqlite3_stmt* pStmt, void* pData)
//{
//	auto pTracks = reinterpret_cast<std::vector<Track>*>(pData);
//
//    Track track;
//    getItemCb(pStmt, &track);
//    pTracks->push_back(std::move(track));
//}

//void MusicDb::searchTracksCb(sqlite3_stmt* pStmt, void* pData)
//{
//    SearchTrackData* pSearchData = reinterpret_cast<SearchTrackData*>(pData);
//
//    Track track;
//    getItemCb(pStmt, &track);
//    pSearchData->subscriber.onItem(track);
//    pSearchData->ids.insert(stringops::toNumeric<uint32_t>(track.albumId));
//}


void MusicDb::addResultCb(sqlite3_stmt* pStmt, void* pData)
{
    assert(sqlite3_column_count(pStmt) == 1);

    uint32_t* pSum = reinterpret_cast<uint32_t*>(pData);
    *pSum += sqlite3_column_int(pStmt, 0);
}

int32_t MusicDb::busyCb(void* pData, int32_t retries)
{
    log::debug("DB busy: attempt %d", retries);
    if (retries > BUSY_RETRIES)
    {
        return 0;
    }

    return 1;
}

}
