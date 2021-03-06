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

#ifndef MUSIC_LIBRARY_H
#define MUSIC_LIBRARY_H

#include <string>
#include <vector>
#include <mutex>

#include "upnp/upnpfwd.h"

namespace doozy
{

class Track;
class Album;

class IMusicLibrary
{
public:
    virtual ~IMusicLibrary() {}

    // returns the total number of objects in the library
    virtual uint32_t getObjectCount() = 0;

    virtual uint32_t getObjectCountInContainer(const std::string& id) = 0;

    virtual upnp::ItemPtr getItem(const std::string& id) = 0;
    virtual std::vector<upnp::ItemPtr> getItems(const std::string& id, uint32_t offset, uint32_t count) = 0;

    virtual void scan(bool startFresh) = 0;
    //virtual void search(const std::string& search, utils::ISubscriber<const Track&>& trackSubscriber, utils::ISubscriber<const Album&>& albumSubscriber) = 0;


    std::function<void()> OnScanComplete;
};

}

#endif
