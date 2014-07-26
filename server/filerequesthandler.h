//    Copyright (C) 2013 Dirk Vanden Boer <dirk.vdb@gmail.com>
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


#ifndef DOOZY_FILE_REQUEST_HANDLER_H
#define DOOZY_FILE_REQUEST_HANDLER_H

#include "upnp/upnpwebserver.h"
#include "library/musicdb.h"
#include "utils/filereader.h"

namespace doozy
{

class FileRequestHandler : public upnp::IVirtualDirCallback
{
public:
    FileRequestHandler(const std::string& dbPath, const std::string& fileUrl);
    
    uint64_t read(uint8_t* buf, uint64_t buflen) override;
    void seekAbsolute(uint64_t position) override;
    void seekRelative(uint64_t offset) override;
    void seekFromEnd(uint64_t offset) override;
    void close() override;
    
private:
    MusicDb             m_db;
    utils::FileReader   m_reader;
};
    
}

#endif
