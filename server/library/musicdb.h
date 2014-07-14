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

#include "library/libraryitem.h"


struct sqlite3;
struct sqlite3_stmt;

namespace doozy
{

class MusicDb
{
public:
    enum TrackStatus
    {
        DoesntExist,
        NeedsUpdate,
        UpToDate
    };

    MusicDb(const std::string& dbFilepath);
    ~MusicDb();

    uint32_t getObjectCount();

    void addItem(const LibraryItem& item);
    void updateItem(const LibraryItem& item);

    bool itemExists(const std::string& filepath);
    TrackStatus getTrackStatus(const std::string& filepath, uint64_t modifiedTime);

    LibraryItem getItem(const std::string& id);
    //Track getTrackWithPath(const std::string& filepath);

    void removeItem(const std::string& id);
    void removeNonExistingFiles();

    //void searchLibrary(const std::string& search, utils::ISubscriber<const Track&>& trackSubscriber, utils::ISubscriber<const Album&>& albumSubscriber);
    void clearDatabase();

private:
    typedef void (*QueryCallback)(sqlite3_stmt*, void*);
    static void getIdCb(sqlite3_stmt* pStmt, void* pData);
    static void getIdIntCb(sqlite3_stmt* pStmt, void* pData);
    static void getItemCb(sqlite3_stmt *pStmt, void *pData);
    static void getTrackModificationTimeCb(sqlite3_stmt* pStmt, void* pData);
    //static void getTracksCb(sqlite3_stmt* pStmt, void* pData);
    static void removeNonExistingFilesCb(sqlite3_stmt* pStmt, void* pData);
    static void countCb(sqlite3_stmt* pStmt, void* pData);
    static void addResultCb(sqlite3_stmt* pStmt, void* pData);
    //static void searchTracksCb(sqlite3_stmt* pStmt, void* pData);

    static int32_t busyCb(void* pData, int32_t retries);

    void getIdFromTable(const std::string& table, const std::string& name, std::string& id);
    uint32_t getIdFromTable(const std::string& table, const std::string& name);
    void createInitialDatabase();
    uint32_t performQuery(sqlite3_stmt* pStmt, QueryCallback cb = nullptr, void* pData = nullptr, bool finalize = true);
    sqlite3_stmt* createStatement(const char* query);
    void bindValue(sqlite3_stmt* pStmt, const std::string& value, int32_t index);
    void bindValue(sqlite3_stmt* pStmt, uint32_t value, int32_t index);
    void bindValue(sqlite3_stmt* pStmt, uint64_t value, int32_t index);
    void bindValue(sqlite3_stmt* pStmt, const void* pData, size_t dataSize, int32_t index);

    sqlite3*                m_pDb;
    std::recursive_mutex    m_DbMutex;
};

}

#endif
