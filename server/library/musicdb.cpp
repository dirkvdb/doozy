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


using namespace std;
using namespace utils;

#define BUSY_RETRIES 50

namespace doozy
{

static std::string getStringFromColumn(sqlite3_stmt* pStmt, int column);

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

    createInitialDatabase();

    utils::trace("Music database loaded");
}

MusicDb::~MusicDb()
{
    if (sqlite3_close(m_pDb) != SQLITE_OK)
    {
        log::error("Failed to close database");
    }
}

uint32_t MusicDb::getObjectCount()
{
    std::lock_guard<std::recursive_mutex> lock(m_DbMutex);
    uint32_t count;
    performQuery(createStatement("SELECT COUNT(Id) FROM objects;"), countCb, &count);

    return count;
}

void MusicDb::addItem(const LibraryItem& item)
{
    assert(item.upnpItem);

    std::lock_guard<std::recursive_mutex> lock(m_DbMutex);
    auto metaId = addMetadata(item);
    
    sqlite3_stmt* pStmt = createStatement(
        "INSERT INTO objects "
        "(Id, ObjectId, ParentId, RefId, Title, Class, MetaData) "
        "VALUES (NULL, ?, ?, ?, ?, ?, ?)");

    bindValue(pStmt, item.upnpItem->getObjectId(), 1);
    bindValue(pStmt, item.upnpItem->getParentId(), 2);
    bindValue(pStmt, item.upnpItem->getRefId(), 3);
    bindValue(pStmt, item.upnpItem->getTitle(), 4);
    bindValue(pStmt, item.upnpItem->getClassString(), 5);
    bindValue(pStmt, metaId, 6);
    performQuery(pStmt);
}

int64_t MusicDb::addMetadata(const LibraryItem& item)
{
    sqlite3_stmt* pStmt = createStatement(
        "INSERT INTO metadata "
        "(Id, ModifiedTime, FilePath) "
        "VALUES (NULL, ?, ?)"
    );
    
    bindValue(pStmt, item.modifiedTime, 1);
    bindValue(pStmt, item.path, 2);
    performQuery(pStmt);
    
    return sqlite3_last_insert_rowid(m_pDb);
}

void MusicDb::updateItem(const LibraryItem& item)
{
    std::lock_guard<std::recursive_mutex> lock(m_DbMutex);
    sqlite3_stmt* pStmt = createStatement(
        "UPDATE objects "
        "SET ParentId=?, RefId=?, Title=?, Class=? "
        "WHERE ObjectId=?"
    );

    bindValue(pStmt, item.upnpItem->getParentId(), 1);
    bindValue(pStmt, item.upnpItem->getRefId(), 2);
    bindValue(pStmt, item.upnpItem->getTitle(), 3);
    bindValue(pStmt, item.upnpItem->getClassString(), 4);
    bindValue(pStmt, item.upnpItem->getObjectId(), 5);
    performQuery(pStmt);
    
    pStmt = createStatement(
        "UPDATE metadata "
        "SET ModifiedTime=? "
        "WHERE FilePath=?"
    );

    bindValue(pStmt, item.modifiedTime, 1);
    bindValue(pStmt, item.path, 2);
    performQuery(pStmt);
}

//void MusicDb::addAlbum(Album& album, AlbumArt& art)
//{
//    {
//        std::lock_guard<std::recursive_mutex> lock(m_DbMutex);
//        if (album.title.empty()) return;
//
//		sqlite3_stmt* pStmt = createStatement(
//			"INSERT INTO albums "
//			"(Id, Name, AlbumArtist, Year, Duration, DiscCount, DateAdded, CoverImage, GenreId) "
//			"VALUES (NULL, ?, ?, ?, ?, ?, ?, ?, ?);");
//
//		string genreId;
//		addGenreIfNotExists(album.genre, genreId);
//
//		bindValue(pStmt, album.title, 1);
//		bindValue(pStmt, album.artist, 2);
//		bindValue(pStmt, album.year, 3);
//		bindValue(pStmt, album.durationInSec, 4);
//		bindValue(pStmt, uint32_t(0), 5);
//		bindValue(pStmt, static_cast<uint64_t>(album.dateAdded), 6);
//		art.getData().empty() ? bindValue(pStmt, "NULL", 7) : bindValue(pStmt, art.getData().data(), art.getData().size(), 7);
//		bindValue(pStmt, genreId, 8);
//
//		performQuery(pStmt);
//
//        getIdFromTable("albums", album.title, album.id);
//    }
//}
//
//void MusicDb::updateAlbum(const Album& album)
//{
//    std::lock_guard<std::recursive_mutex> lock(m_DbMutex);
//    sqlite3_stmt* pStmt = createStatement(
//        "UPDATE albums "
//        "SET Name=?, AlbumArtist=?, Year=?, Duration=?, GenreId=?"
//        "WHERE Id=?;"
//    );
//
//    string genreId;
//    addGenreIfNotExists(album.genre, genreId);
//
//    bindValue(pStmt, album.title, 1);
//    bindValue(pStmt, album.artist, 2);
//    bindValue(pStmt, album.year, 3);
//    bindValue(pStmt, album.durationInSec, 4);
//    bindValue(pStmt, genreId, 5);
//    bindValue(pStmt, album.id, 6);
//
//    performQuery(pStmt);
//}

bool MusicDb::itemExists(const string& filepath)
{
    std::lock_guard<std::recursive_mutex> lock(m_DbMutex);
    sqlite3_stmt* pStmt = createStatement(
        "SELECT Id FROM metadata "
        "WHERE metadata.Filepath = ?"
    );
    
    if (sqlite3_bind_text(pStmt, 1, filepath.c_str(), static_cast<int>(filepath.size()), SQLITE_STATIC) != SQLITE_OK)
    {
        throw runtime_error(string("Failed to bind value: ") + sqlite3_errmsg(m_pDb));
    }

    return performQuery(pStmt) == 1;
}

ItemStatus MusicDb::getItemStatus(const std::string& filepath, uint64_t modifiedTime)
{
    std::lock_guard<std::recursive_mutex> lock(m_DbMutex);
    sqlite3_stmt* pStmt = createStatement("SELECT ModifiedTime FROM metadata WHERE metadata.Filepath = ?");
    if (sqlite3_bind_text(pStmt, 1, filepath.c_str(), static_cast<int>(filepath.size()), SQLITE_STATIC) != SQLITE_OK)
    {
        throw runtime_error(string("Failed to bind value: ") + sqlite3_errmsg(m_pDb));
    }

    uint32_t dbModifiedTime;
    int32_t numTracks = performQuery(pStmt, getTrackModificationTimeCb, &dbModifiedTime);
    assert (numTracks <= 1);

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

LibraryItem MusicDb::getItem(const std::string& id)
{
    std::lock_guard<std::recursive_mutex> lock(m_DbMutex);
    sqlite3_stmt* pStmt = createStatement(
        "SELECT objects.ObjectId, objects.Title, objects.ParentId, objects.RefId, objects.Class, metadata.ModifiedTime, metadata.FilePath, Objects.MetaData "
        "FROM objects "
        "LEFT OUTER JOIN metadata ON objects.MetaData = metadata.Id "
        "WHERE objects.ObjectId = ? ");

    LibraryItem item;
    bindValue(pStmt, id, 1);

    if (performQuery(pStmt, getItemCb, &item) == 0 || item.upnpItem->getObjectId().empty())
    {
        throw std::runtime_error("No track found in db with id: " + id);
    }

    return item;
}

std::vector<LibraryItem> MusicDb::getItems(const std::string& parentId, uint32_t offset, uint32_t count)
{
    std::lock_guard<std::recursive_mutex> lock(m_DbMutex);
    sqlite3_stmt* pStmt = createStatement(
        "SELECT objects.ObjectId, objects.Title, objects.ParentId, objects.RefId, objects.Class, metadata.ModifiedTime, metadata.FilePath , Objects.MetaData "
        "FROM objects "
        "LEFT OUTER JOIN metadata ON objects.MetaData = metadata.Id "
        "WHERE objects.ParentId = ? LIMIT ? OFFSET ?");

    std::vector<LibraryItem> items;
    bindValue(pStmt, parentId, 1);
    bindValue(pStmt, count == 0 ? -1 : count, 2);
    bindValue(pStmt, offset, 3);

    performQuery(pStmt, getItemsCb, &items);
    return items;
}

//Track MusicDb::getTrackWithPath(const string& filepath)
//{
//    std::lock_guard<std::recursive_mutex> lock(m_DbMutex);
//    sqlite3_stmt* pStmt = createStatement(
//        "SELECT tracks.Id, tracks.albumId, tracks.Title, tracks.Composer, tracks.Filepath, tracks.Year, tracks.TrackNr, tracks.DiscNr, tracks.Duration, tracks.BitRate, tracks.SampleRate, tracks.Channels, tracks.FileSize, tracks.ModifiedTime, artists.Name, albums.Name, albums.AlbumArtist, genres.Name "
//        "FROM tracks "
//        "LEFT OUTER JOIN albums ON tracks.AlbumId = albums.Id "
//        "LEFT OUTER JOIN artists ON tracks.ArtistId = artists.Id "
//        "LEFT OUTER JOIN genres ON tracks.GenreId = genres.Id "
//        "WHERE tracks.Filepath = ? ;");
//
//    Track track;
//    bindValue(pStmt, filepath, 1);
//    performQuery(pStmt, getItemCb, &track);
//
//    if (track.id.empty())
//    {
//        throw std::runtime_error("No track found in db with path: " + filepath);
//    }
//
//    return track;
//}

void MusicDb::removeItem(const std::string& id)
{
    std::lock_guard<std::recursive_mutex> lock(m_DbMutex);
    sqlite3_stmt* pStmt = createStatement("DELETE from objects WHERE Id = ?");
    bindValue(pStmt, id, 1);
    performQuery(pStmt);
}

void MusicDb::removeMetaData(const std::string& id)
{
    std::lock_guard<std::recursive_mutex> lock(m_DbMutex);
    sqlite3_stmt* pStmt = createStatement("DELETE from metadata WHERE Id = ?");
    bindValue(pStmt, id, 1);
    performQuery(pStmt);
}


void MusicDb::removeNonExistingFiles()
{
    std::lock_guard<std::recursive_mutex> lock(m_DbMutex);
    std::vector<std::pair<uint32_t, uint32_t>> files;
    performQuery(createStatement(
        "SELECT objects.Id, objects.MetaData, metadata.FilePath "
        "FROM objects "
        "LEFT OUTER JOIN metadata ON objects.MetaData = metadata.Id"
        ), removeNonExistingFilesCb, &files);

    for (size_t i = 0; i < files.size(); ++i)
    {
        log::debug("Removed deleted file from database: %d", files[i].first);
        removeItem(numericops::toString(files[i].first));
        removeMetaData(numericops::toString(files[i].second));
    }
}

void MusicDb::removeNonExistingFilesCb(sqlite3_stmt* pStmt, void* pData)
{
    assert(sqlite3_column_count(pStmt) == 3);

    auto& ids = *reinterpret_cast<std::vector<std::pair<uint32_t, uint32_t>>*>(pData);

    string path = getStringFromColumn(pStmt, 2);
    if (!path.empty() && !fileops::pathExists(path))
    {
        uint32_t objectsId = sqlite3_column_int(pStmt, 0);
        uint32_t metaDataId = sqlite3_column_int(pStmt, 1);
        ids.push_back(std::make_pair(objectsId, metaDataId));
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
//    std::lock_guard<std::recursive_mutex> lock(m_DbMutex);
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
    std::lock_guard<std::recursive_mutex> lock(m_DbMutex);
    performQuery(createStatement("DROP INDEX IF EXISTS tracks.pathIndex;"));
    performQuery(createStatement("DROP TABLE IF EXISTS objects;"));
    performQuery(createStatement("DROP TABLE IF EXISTS metadata;"));

    createInitialDatabase();
}

void MusicDb::createInitialDatabase()
{
    log::debug("Create initial database");
    std::lock_guard<std::recursive_mutex> lock(m_DbMutex);
    performQuery(createStatement("CREATE TABLE IF NOT EXISTS objects("
        "Id INTEGER PRIMARY KEY,"
        "ObjectId TEXT UNIQUE,"
        "ParentId TEXT,"
        "RefId TEXT,"
        "Title TEXT,"
        "Class TEXT,"
        "MetaData INTEGER,"
        "FOREIGN KEY (MetaData) REFERENCES metadata(id));"));

    performQuery(createStatement("CREATE TABLE IF NOT EXISTS metadata("
        "Id INTEGER PRIMARY KEY,"
        "Album TEXT,"
        "Artist TEXT,"
        "AlbumArtist TEXT"
        "Title TEXT,"
        "Genre TEXT,"
        "Composer TEXT,"
        "Year INTEGER,"
        "TrackNr INTEGER,"
        "DiscNr INTEGER,"
        "AlbumOrder INTEGER,"
        "Duration TEXT,"
        "MimeType TEXT,"
        "BitRate INTEGER,"
        "SampleRate INTEGER,"
        "Channels INTEGER,"
        "FileSize INTEGER,"
        "DateAdded INTEGER,"
        "ModifiedTime INTEGER,"
        "FilePath TEXT,"
        "CoverImage BLOB);"));

    log::debug("database created");
}

uint32_t MusicDb::performQuery(sqlite3_stmt* pStmt, QueryCallback cb, void* pData, bool finalize)
{
    uint32_t rowCount = 0;

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
            if (cb != nullptr) cb(pStmt, pData);
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

void MusicDb::bindValue(sqlite3_stmt* pStmt, const string& value, int32_t index)
{
    if (value.empty())
    {
        if (sqlite3_bind_null(pStmt, index) != SQLITE_OK)
        {
            throw runtime_error(string("Failed to bind string value as NULL: ") + sqlite3_errmsg(m_pDb));
        }
    }
    else if (sqlite3_bind_text(pStmt, index, value.c_str(), static_cast<int>(value.size()), SQLITE_STATIC) != SQLITE_OK)
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

void MusicDb::getIdFromTable(const string& table, const string& name, string& id)
{
    stringstream query;
    query << "SELECT Id FROM " << table << " WHERE Name = ?;";
    sqlite3_stmt* pStmt = createStatement(("SELECT Id FROM " + table + " WHERE Name = ?;").c_str());
    bindValue(pStmt, name, 1);

    id.clear();
    performQuery(pStmt, getIdCb, &id);
}

uint32_t MusicDb::getIdFromTable(const string& table, const string& name)
{
    stringstream query;
    query << "SELECT Id FROM " << table << " WHERE ObjectId = ?;";
    sqlite3_stmt* pStmt = createStatement(query.str().c_str());
    bindValue(pStmt, name, 1);

    uint32_t id = 0;
    performQuery(pStmt, getIdIntCb, &id);
    return id;
}

void MusicDb::getIdCb(sqlite3_stmt* pStmt, void* pData)
{
    assert(sqlite3_column_count(pStmt) == 1);
    assert(sqlite3_column_text(pStmt, 0));

    string* pId = reinterpret_cast<string*>(pData);
    *pId = reinterpret_cast<const char*>(sqlite3_column_text(pStmt, 0));
}

void MusicDb::getIdIntCb(sqlite3_stmt* pStmt, void* pData)
{
    assert(sqlite3_column_count(pStmt) == 1);
    assert(sqlite3_column_text(pStmt, 0));

    uint32_t* pId = reinterpret_cast<uint32_t*>(pData);
    *pId = sqlite3_column_int(pStmt, 0);
}

static std::string getStringFromColumn(sqlite3_stmt* pStmt, int column)
{
    const char* pString = reinterpret_cast<const char*>(sqlite3_column_text(pStmt, column));
    if (pString != nullptr)
    {
        return pString;
    }
    
    return std::string();
}

void MusicDb::getItemCb(sqlite3_stmt *pStmt, void *pData)
{
    assert(sqlite3_column_count(pStmt) == 8);

    auto& item = *reinterpret_cast<LibraryItem*>(pData);

    item.upnpItem = std::make_shared<upnp::Item>();
    item.upnpItem->setObjectId(getStringFromColumn(pStmt, 0));
    item.upnpItem->setTitle(getStringFromColumn(pStmt, 1));
    item.upnpItem->setParentId(getStringFromColumn(pStmt, 2));
    item.upnpItem->setRefId(getStringFromColumn(pStmt, 3));
    item.upnpItem->setClass(getStringFromColumn(pStmt, 4));
    item.modifiedTime = sqlite3_column_int64(pStmt, 5);
    item.path = getStringFromColumn(pStmt, 6);
}

void MusicDb::getItemsCb(sqlite3_stmt *pStmt, void *pData)
{
    assert(sqlite3_column_count(pStmt) == 8);

    auto& items = *reinterpret_cast<std::vector<LibraryItem>*>(pData);
    
    LibraryItem item;
    getItemCb(pStmt, &item);
    items.push_back(item);
}

void MusicDb::getTrackModificationTimeCb(sqlite3_stmt* pStmt, void* pData)
{
    assert(sqlite3_column_count(pStmt) == 1);

    uint64_t* pModifiedTime = reinterpret_cast<uint64_t*>(pData);
    *pModifiedTime = sqlite3_column_int64(pStmt, 0);
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

static void getDataFromColumn(sqlite3_stmt* pStmt, int column, vector<uint8_t>& data)
{
	int type = sqlite3_column_type(pStmt, column);
	if (type == SQLITE_BLOB)
	{
		const void* dataPtr = sqlite3_column_blob(pStmt, column);
		int size = sqlite3_column_bytes(pStmt, column);
		if (size == 0)
		{
			return;
		}

		data.resize(size);
		memcpy(&data[0], dataPtr, size);
	}
}

void MusicDb::countCb(sqlite3_stmt* pStmt, void* pData)
{
    assert(sqlite3_column_count(pStmt) == 1);

    uint32_t* pCount = reinterpret_cast<uint32_t*>(pData);
    *pCount = sqlite3_column_int(pStmt, 0);
}

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
