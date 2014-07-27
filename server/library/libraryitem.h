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

#ifndef LIBRARY_ITEM_H
#define LIBRARY_ITEM_H

#include <string>

namespace doozy
{

struct LibraryItem
{
    std::string     objectId;
    std::string     parentId;
    std::string     refId;
    std::string     name;
    std::string     artist;
    std::string     title;
    std::string     album;
    std::string     genre;
    std::string     date;
    std::string     upnpClass;
    std::string     path;
    uint32_t        trackNumber;
    uint32_t        discNumber;
    uint64_t        fileSize = 0;
    uint64_t        modifiedTime = 0;
};

}

#endif
