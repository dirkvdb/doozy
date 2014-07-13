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

#ifndef SCANNER_H
#define SCANNER_H

#include <string>
#include <vector>
#include "utils/threadpool.h"

namespace doozy
{

class Track;
class Album;
class AlbumArt;
class MusicDb;

class Scanner
{
public:
    Scanner(MusicDb& db, const std::vector<std::string>& albumArtFilenames);
    ~Scanner();

    void performScan(const std::string& libraryPath);
    void cancel();

private:
    void scan(const std::string& dir);
    void onFile(const std::string& filepath);
    void processAlbumArt(const std::string& filepath, AlbumArt& art);

    MusicDb&                        m_LibraryDb;
    int32_t                         m_ScannedFiles;
    std::vector<std::string>        m_AlbumArtFilenames;
    utils::ThreadPool               m_ThreadPool;

    bool                            m_InitialScan;
    bool                            m_Stop;
};

}

#endif
