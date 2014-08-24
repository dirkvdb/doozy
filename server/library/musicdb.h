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

#ifndef MUSIC_DB_H
#define MUSIC_DB_H

#include <string>
#include <vector>
#include <mutex>

#include "upnp/upnpfwd.h"
#include "library/libraryitem.h"
#include <sqlpp11/sqlite3/sqlite3.h>

namespace sqlpp
{
namespace sqlite3
{

class connection;

}
}

namespace doozy
{

enum class ItemStatus
{
    DoesntExist,
    NeedsUpdate,
    UpToDate
};

class MusicDb
{
public:
    MusicDb(const std::string& dbFilepath);
    ~MusicDb();

    void setWebRoot(const std::string& webRoot);

    uint64_t getObjectCount();
    uint64_t getChildCount(const std::string& id);
    uint64_t getUniqueIdInContainer(const std::string& containerId);

    void addItem(const LibraryItem& item);
    void addItem(const LibraryItem& item, const LibraryMetadata& meta);
    void addItems(const std::vector<std::pair<LibraryItem, LibraryMetadata>>& items);
    void addItems(const std::vector<std::pair<std::vector<LibraryItem>, LibraryMetadata>>& items);
    void updateItem(const LibraryItem& item, const LibraryMetadata& meta);

    bool itemExists(const std::string& filepath, std::string& objectId);
    bool albumExists(const std::string& title, const std::string& artist, std::string& objectId);
    ItemStatus getItemStatus(const std::string& filepath, uint64_t modifiedTime);
    std::string getItemPath(const std::string& id);

    upnp::ItemPtr getItem(const std::string& id);
    std::vector<upnp::ItemPtr> getItems(const std::string& parentId, uint32_t offset, uint32_t count);

    void removeItem(const std::string& id);
    void removeNonExistingFiles();

    //void searchLibrary(const std::string& search, utils::ISubscriber<const Track&>& trackSubscriber, utils::ISubscriber<const Album&>& albumSubscriber);
    void clearDatabase();

private:
    template <typename T>
    upnp::ItemPtr parseItem(const T& row);

    //static void searchTracksCb(sqlite3_stmt* pStmt, void* pData);

    int64_t addItem(const LibraryItem& item, int64_t metaId);
    int64_t addMetadata(const LibraryMetadata& meta);
    void removeMetaData(int64_t id);
    void createInitialDatabase();

    void prepareStatements();

    sqlpp::sqlite3::connection                  m_db;
    struct PreparedStatements;
    std::unique_ptr<PreparedStatements>         m_statements;
    std::string                                 m_webRoot;
};

}

#endif
