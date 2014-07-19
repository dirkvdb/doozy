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

#include "mimetypes.h"

#include "Utils/fileoperations.h"
#include "Utils/stringoperations.h"

namespace doozy
{
namespace mime
{

using namespace utils;

Group groupFromFile(const std::string& filePath)
{
    switch (typeFromFile(filePath))
    {
    case Type::AudioMp3:
    case Type::AudioM4a:
    case Type::AudioWma:
    case Type::AudioFlac:
    case Type::AudioWave:
    case Type::AudioPcm:
    case Type::AudioOgg:
        return Group::Audio;

    case Type::VideoAvi:
    case Type::VideoMpeg:
    case Type::VideoMp4:
    case Type::VideoWmv:
    case Type::VideoMkv:
    case Type::VideoFlv:
    case Type::VideoMov:
    case Type::Video3gp:
    case Type::VideoTivo:
        return Group::Video;

    case Type::ImageJpeg:
    case Type::ImageTiff:
    case Type::ImageBmp:
        return Group::Image;
    default:
        return Group::Other;
    }
}

Type typeFromFile(const std::string& filePath)
{
    auto ext = stringops::lowercase(fileops::getFileExtension(filePath));
    
         if (ext == "mp3")  return Type::AudioMp3;
    else if (ext == "m4a")  return Type::AudioM4a;
    else if (ext == "wma")  return Type::AudioWma;
    else if (ext == "flac") return Type::AudioFlac;
    else if (ext == "wav")  return Type::AudioWave;
    else if (ext == "pcm")  return Type::AudioPcm;
    else if (ext == "ogg")  return Type::AudioOgg;
    
    else if (ext == "avi")  return Type::VideoAvi;
    else if (ext == "mpg")  return Type::VideoMpeg;
    else if (ext == "mpeg") return Type::VideoMpeg;
    else if (ext == "mp4")  return Type::VideoMp4;
    else if (ext == "wmv")  return Type::VideoWmv;
    else if (ext == "mkv")  return Type::VideoMkv;
    else if (ext == "flv")  return Type::VideoFlv;
    else if (ext == "mov")  return Type::VideoMov;
    else if (ext == "3gp")  return Type::Video3gp;
    else if (ext == "tivo") return Type::VideoTivo;
    
    else if (ext == "jpg")  return Type::ImageJpeg;
    else if (ext == "jpeg") return Type::ImageJpeg;
    else if (ext == "tiff") return Type::ImageTiff;
    else if (ext == "bmp")  return Type::ImageBmp;
    
    return Type::Other;
}

}
}
