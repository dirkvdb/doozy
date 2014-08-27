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

#ifndef DOOZY_DATABASE_H
#define DOOZY_DATABASE_H

#include <string>
#include <vector>
#include <mutex>
#include <type_traits>
#include <typeinfo>

#include "musicdb.h"

namespace doozy
{

enum class ThreadingModel
{
    SingleThreaded,
    MultiThreaded
};

template <ThreadingModel threadingModel = ThreadingModel::SingleThreaded>
class Database
{

class NoMutex
{
public:
    void lock() {}
    void unlock() {}
    void try_lock() {}
};

typedef typename std::conditional<threadingModel == ThreadingModel::SingleThreaded, NoMutex, std::mutex>::type MutexType;

public:
    Database(const std::string& dbFilepath) : m_db(dbFilepath) {}

    template <typename... Args>
    auto setWebRoot(Args&&... args)
    {
        std::lock_guard<MutexType> lock(m_mutex);
        return m_db.setWebRoot(std::forward<Args>(args)...);
    }

    template <typename... Args>
    auto getObjectCount(Args&&... args)
    {
        std::lock_guard<MutexType> lock(m_mutex);
        return m_db.getObjectCount(std::forward<Args>(args)...);
    }
    
    template <typename... Args>
    auto getChildCount(Args&&... args)
    {
        std::lock_guard<MutexType> lock(m_mutex);
        return m_db.getChildCount(std::forward<Args>(args)...);
    }
    
    template <typename... Args>
    auto addItemWithId(Args&&... args)
    {
        std::lock_guard<MutexType> lock(m_mutex);
        return m_db.addItemWithId(std::forward<Args>(args)...);
    }

    template <typename... Args>
    auto addItem(Args&&... args)
    {
        std::lock_guard<MutexType> lock(m_mutex);
        return m_db.addItem(std::forward<Args>(args)...);
    }
    
    template <typename... Args>
    auto addItems(Args&&... args)
    {
        std::lock_guard<MutexType> lock(m_mutex);
        return m_db.addItems(std::forward<Args>(args)...);
    }
    
    template <typename... Args>
    auto updateItem(Args&&... args)
    {
        std::lock_guard<MutexType> lock(m_mutex);
        return m_db.updateItem(std::forward<Args>(args)...);
    }
    
    template <typename... Args>
    auto itemExists(Args&&... args)
    {
        std::lock_guard<MutexType> lock(m_mutex);
        return m_db.itemExists(std::forward<Args>(args)...);
    }
    
    template <typename... Args>
    auto albumExists(Args&&... args)
    {
        std::lock_guard<MutexType> lock(m_mutex);
        return m_db.albumExists(std::forward<Args>(args)...);
    }
    
    template <typename... Args>
    auto getItemStatus(Args&&... args)
    {
        std::lock_guard<MutexType> lock(m_mutex);
        return m_db.getItemStatus(std::forward<Args>(args)...);
    }
    
    template <typename... Args>
    auto getItemPath(Args&&... args)
    {
        std::lock_guard<MutexType> lock(m_mutex);
        return m_db.getItemPath(std::forward<Args>(args)...);
    }
    
    template <typename... Args>
    auto getItem(Args&&... args)
    {
        std::lock_guard<MutexType> lock(m_mutex);
        return m_db.getItem(std::forward<Args>(args)...);
    }
    
    template <typename... Args>
    auto getItems(Args&&... args)
    {
        std::lock_guard<MutexType> lock(m_mutex);
        return m_db.getItems(std::forward<Args>(args)...);
    }
    

    template <typename... Args>
    auto removeItem(Args&&... args)
    {
        std::lock_guard<MutexType> lock(m_mutex);
        return m_db.removeItem(std::forward<Args>(args)...);
    }
    
    template <typename... Args>
    auto removeNonExistingFiles(Args&&... args)
    {
        std::lock_guard<MutexType> lock(m_mutex);
        return m_db.removeNonExistingFiles(std::forward<Args>(args)...);
    }
    
    template <typename... Args>
    auto clearDatabase(Args&&... args)
    {
        std::lock_guard<MutexType> lock(m_mutex);
        return m_db.clearDatabase(std::forward<Args>(args)...);
    }
    
private:
    MusicDb     m_db;
    MutexType   m_mutex;
};

}

#endif
