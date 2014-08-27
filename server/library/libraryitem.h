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
    int64_t         objectId = 0;
    int64_t         parentId = 0;
    int64_t         refId = 0;
    std::string     name;
    std::string     upnpClass;
};

struct LibraryMetadata
{
    std::string     artist;
    std::string     title;
    std::string     album;
    std::string     albumArtist;
    std::string     genre;
    std::string     date;
    std::string     path;
    std::string     mimeType;
    std::string     thumbnail;
    uint32_t        duration;
    uint32_t        nrChannels;
    uint32_t        bitrate;
    uint32_t        sampleRate;
    uint32_t        trackNumber;
    uint32_t        discNumber;
    uint64_t        fileSize = 0;
    uint64_t        modifiedTime = 0;
};

}

#endif
