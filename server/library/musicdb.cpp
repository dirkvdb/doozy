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
#include <sqlpp11/sqlite3/sqlite3.h>

#include "upnp/upnpitem.h"
#include "mimetypes.h"
#include "doozyscheme.h"

using namespace std;
using namespace utils;

#define BUSY_RETRIES 50

namespace doozy
{

namespace sql = sqlpp::sqlite3;

namespace
{

doozy::Objects objects;
doozy::Metadata metadata;

SQLPP_ALIAS_PROVIDER(numObjects);

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

template <typename T>
bool getIdFromResultIfExists(const T& result, std::string& id)
{
    bool hasResults = !result.empty();
    if (hasResults)
    {
        id = result.front().ObjectId;
    }
    
    return hasResults;
}

}

MusicDb::MusicDb(const string& dbFilepath)
: m_pDb(nullptr)
{
    utils::trace("Create Music database");
    
    auto config = std::make_shared<sql::connection_config>();
    config->path_to_database = dbFilepath;
    config->flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    config->debug = true;
    
    m_db.reset(new sql::connection(config));

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
    return m_db->run(
        select(count(objects.Id).as(numObjects))
        .from(objects)
        .where(true)
    ).front().numObjects;
}

uint64_t MusicDb::getChildCount(const std::string& id)
{
    return m_db->run(
        select(count(objects.Id).as(numObjects))
        .from(objects)
        .where(objects.ParentId == id)
    ).front().numObjects;
}

uint64_t MusicDb::getUniqueIdInContainer(const std::string& containerId)
{
    return m_db->run(
        select(count(objects.Id).as(numObjects))
        .from(objects)
        .where(objects.ParentId == containerId)
    ).front().numObjects;
}

void MusicDb::addItem(const LibraryItem& item)
{
    auto metaId = addMetadata(item);

    auto insertion = insert_into(objects).columns(objects.ObjectId, objects.ParentId, objects.RefId, objects.Name, objects.Class, objects.MetaData);
    insertion.values.add(objects.ObjectId   = item.objectId,
                         objects.ParentId   = sqlpp::tvin(item.parentId),
                         objects.RefId      = sqlpp::tvin(item.refId),
                         objects.Name       = item.name,
                         objects.Class      = sqlpp::tvin(item.upnpClass),
                         objects.MetaData   = metaId);
    
    m_db->run(insertion);
}

void MusicDb::addItems(const std::vector<LibraryItem>& items)
{
    std::lock_guard<std::recursive_mutex> lock(m_dbMutex);
    
//    sqlite3_stmt* pMetaStmt = createStatement(
//        "INSERT INTO metadata "
//        "(Id, ModifiedTime, FilePath, FileSize, Title, Artist, Genre, MimeType, Duration, Channels, BitRate, SampleRate, Thumbnail) "
//        "VALUES (NULL, ?,   ?,        ?,        ?,     ?,      ?,     ?,        ?,        ?,        ?,       ?,          ?)");
//    
//    sqlite3_stmt* pStmt = createStatement(
//        "INSERT INTO objects "
//        "(Id, ObjectId, ParentId, RefId, Name, Class, MetaData) "
//        "VALUES (NULL, ?, ?, ?, ?, ?, last_insert_rowid())");
    
    auto preparedItemAdd = m_db->prepare(
        insert_into(objects).set(
            objects.ObjectId    = parameter(objects.ObjectId),
            objects.ParentId    = parameter(objects.ParentId),
            objects.RefId       = parameter(objects.RefId),
            objects.Name        = parameter(objects.Name),
            objects.Class       = parameter(objects.Class),
            objects.MetaData    = sqlpp::verbatim<sqlpp::integer>("last_insert_rowid()")
        )
    );

    //m_db->execute("BEGIN");
    //performQuery(m_pBeginStatement, false);
    for (auto& item : items)
    {
        addMetadata(item);

        preparedItemAdd.params.ObjectId = item.objectId;
        preparedItemAdd.params.ParentId = item.parentId;
        preparedItemAdd.params.RefId    = item.refId;
        preparedItemAdd.params.Name     = item.name;
        preparedItemAdd.params.Class    = item.upnpClass;
        m_db->run(preparedItemAdd);
    }
    
    //performQuery(m_pCommitStatement, false);
    //m_db->execute("END");
    
    //sqlite3_finalize(pStmt);
    //sqlite3_finalize(pMetaStmt);
}

int64_t MusicDb::addMetadata(const LibraryItem& item)
{
    auto insertion = insert_into(metadata).columns(metadata.ModifiedTime,
                                                   metadata.FilePath,
                                                   metadata.FileSize,
                                                   metadata.Title,
                                                   metadata.Artist,
                                                   metadata.Genre,
                                                   metadata.MimeType,
                                                   metadata.Duration,
                                                   metadata.Channels,
                                                   metadata.BitRate,
                                                   metadata.SampleRate,
                                                   metadata.Thumbnail);
    
    insertion.values.add(metadata.ModifiedTime  = static_cast<int64_t>(item.modifiedTime),
                         metadata.FilePath      = sqlpp::tvin(item.path),
                         metadata.FileSize      = static_cast<int64_t>(item.fileSize),
                         metadata.Title         = sqlpp::tvin(item.title),
                         metadata.Artist        = sqlpp::tvin(item.artist),
                         metadata.Genre         = sqlpp::tvin(item.genre),
                         metadata.MimeType      = sqlpp::tvin(item.mimeType),
                         metadata.Duration      = item.duration,
                         metadata.Channels      = item.nrChannels,
                         metadata.BitRate       = item.bitrate,
                         metadata.SampleRate    = item.sampleRate,
                         metadata.Thumbnail     = sqlpp::tvin(item.thumbnail));
    
    return m_db->run(insertion);
}

void MusicDb::updateItem(const LibraryItem& item)
{
    m_db->run(update(objects).set(
        objects.ObjectId   = item.objectId,
        objects.ParentId   = sqlpp::tvin(item.parentId),
        objects.RefId      = sqlpp::tvin(item.refId),
        objects.Name       = item.name,
        objects.Class      = sqlpp::tvin(item.upnpClass)
    ).where(true));
    
    m_db->run(update(metadata).set(
        metadata.ModifiedTime  = static_cast<int64_t>(item.modifiedTime),
        metadata.FilePath      = sqlpp::tvin(item.path),
        metadata.FileSize      = static_cast<int64_t>(item.fileSize),
        metadata.Title         = sqlpp::tvin(item.title),
        metadata.Artist        = sqlpp::tvin(item.artist),
        metadata.Genre         = sqlpp::tvin(item.genre),
        metadata.MimeType      = sqlpp::tvin(item.mimeType),
        metadata.Duration      = item.duration,
        metadata.Channels      = item.nrChannels,
        metadata.BitRate       = item.bitrate,
        metadata.SampleRate    = item.sampleRate,
        metadata.Thumbnail     = sqlpp::tvin(item.thumbnail)
    ).where(true));
}

bool MusicDb::itemExists(const string& filepath, string& objectId)
{
    const auto& result = m_db->run(
        select(objects.ObjectId)
        .from(metadata.left_outer_join(objects).on(objects.MetaData == metadata.Id))
        .where(metadata.FilePath == filepath)
    );
    
    return getIdFromResultIfExists(result, objectId);
}

bool MusicDb::albumExists(const std::string& title, const std::string& artist, string& objectId)
{
    auto selectFrom = select(objects.ObjectId)
                      .from(metadata.left_outer_join(objects).on(objects.MetaData == metadata.Id));

    if (artist.empty())
    {
        const auto& result = m_db->run(
            selectFrom.where(objects.Class == "object.container.album.musicAlbum" and objects.Name == title and metadata.Artist.is_null())
        );
        
        return getIdFromResultIfExists(result, objectId);
    }
    else
    {
        const auto& result = m_db->run(
            selectFrom.where(objects.Class == "object.container.album.musicAlbum" and objects.Name == title and metadata.Artist == artist)
        );
        
        return getIdFromResultIfExists(result, objectId);
    }
}

ItemStatus MusicDb::getItemStatus(const std::string& filepath, uint64_t modifiedTime)
{
    const auto& result = m_db->run(
        select(metadata.ModifiedTime)
        .from(metadata)
        .where(metadata.FilePath == filepath)
    );
    
    if (result.empty())
    {
        return ItemStatus::DoesntExist;
    }
    
    return result.front().ModifiedTime < modifiedTime ? ItemStatus::NeedsUpdate : ItemStatus::UpToDate;
}

template <typename T>
upnp::ItemPtr MusicDb::parseItem(const T& row)
{
    assert(!m_webRoot.empty());

    auto item = std::make_shared<upnp::Item>();
    item->setObjectId(row.ObjectId);
    item->setTitle(row.Name);
    item->setParentId(row.ParentId);
    item->setRefId(row.RefId);
    item->setClass(row.Class);
    item->addMetaData(upnp::Property::Artist, row.Artist);

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

upnp::ItemPtr MusicDb::getItem(const std::string& id)
{
    const auto& result = m_db->run(
        select(objects.ObjectId, objects.Name, objects.ParentId, objects.RefId, objects.Class, all_of(metadata))
        .from(objects.left_outer_join(metadata).on(objects.MetaData == metadata.Id))
        .where(objects.ObjectId == id)
    );

    if (result.empty())
    {
        throw std::runtime_error("No track found in db with id: " + id);
    }

    return parseItem(result.front());
}

std::vector<upnp::ItemPtr> MusicDb::getItems(const std::string& parentId, uint32_t offset, uint32_t count)
{
    std::vector<upnp::ItemPtr> items;

    for (const auto& row : m_db->run(
        select(objects.ObjectId, objects.Name, objects.ParentId, objects.RefId, objects.Class, all_of(metadata))
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

std::string MusicDb::getItemPath(const std::string& id)
{
    const auto& result = m_db->run(
        select(metadata.FilePath)
        .from(objects.left_outer_join(metadata).on(objects.MetaData == metadata.Id))
        .where(objects.ObjectId == id)
    );
    
    if (result.empty())
    {
        throw std::runtime_error("No item in database with id: " + id);
    }

    return result.front().FilePath;
}

void MusicDb::removeItem(const std::string& id)
{
    m_db->run(
        remove_from(objects)
        .where(objects.ObjectId == id)
    );
}

void MusicDb::removeMetaData(int64_t id)
{
    m_db->run(
        remove_from(metadata)
        .where(metadata.Id == static_cast<int64_t>(id))
    );
}


void MusicDb::removeNonExistingFiles()
{
    std::vector<std::pair<std::string, int64_t>> files;

    for (const auto& row : m_db->run(select(objects.ObjectId, objects.MetaData, metadata.FilePath)
                                     .from(objects.left_outer_join(metadata).on(objects.MetaData == metadata.Id))
                                     .where(true)))
    {
        std::string path = row.FilePath;
        if (!path.empty() && !fileops::pathExists(path))
        {
            files.emplace_back(row.ObjectId, row.MetaData);
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
    m_db->execute("DROP INDEX IF EXISTS metadata.pathIndex;");
    m_db->execute("DROP TABLE IF EXISTS objects;");
    m_db->execute("DROP TABLE IF EXISTS metadata;");

    createInitialDatabase();
}

void MusicDb::createInitialDatabase()
{
    m_db->execute("CREATE TABLE IF NOT EXISTS objects("
        "Id INTEGER PRIMARY KEY,"
        "ObjectId TEXT UNIQUE,"
        "ParentId TEXT,"
        "RefId TEXT,"
        "Name TEXT NOT NULL,"
        "Class TEXT,"
        "MetaData INTEGER,"
        "FOREIGN KEY (MetaData) REFERENCES metadata(id));");

    m_db->execute("CREATE TABLE IF NOT EXISTS metadata("
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

    m_db->execute("CREATE INDEX IF NOT EXISTS pathIndex ON metadata (FilePath);");
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
