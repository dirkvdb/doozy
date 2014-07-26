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

#include "filerequesthandler.h"

#include "utils/fileoperations.h"

using namespace utils;

namespace doozy
{

FileRequestHandler::FileRequestHandler(const std::string& dbPath, const std::string& fileUrl)
: m_db(dbPath)
{
    auto id = fileops::getFileName(fileUrl);
    auto path = m_db.getItemPath(id);
    
    m_reader.open(path);
}

uint64_t FileRequestHandler::read(uint8_t* buf, uint64_t buflen)
{
    return m_reader.read(buf, buflen);
}

void FileRequestHandler::seekRelative(uint64_t offset)
{
    m_reader.seekRelative(offset);
}


void FileRequestHandler::seekAbsolute(uint64_t position)
{
    m_reader.seekAbsolute(position);
}

void FileRequestHandler::seekFromEnd(uint64_t offset)
{
    m_reader.seekAbsolute(m_reader.getContentLength() - offset);
}

void FileRequestHandler::close()
{
    m_reader.close();
}

}
