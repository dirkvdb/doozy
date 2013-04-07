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

#include "playqueue.h"

#include "utils/log.h"
#include "utils/fileoperations.h"
#include "utils/stringoperations.h"
#include "upnp/upnphttpclient.h"
#include "audio/audiom3uparser.h"

#include <fstream>
#include <cassert>

using namespace std;
using namespace utils;

namespace doozy
{

PlayQueue::PlayQueue()
: m_Destroy(false)
{
}

PlayQueue::~PlayQueue()
{
    m_Destroy = true;
}

void PlayQueue::addTrack(const std::string& trackUri)
{
    const std::string extension = fileops::getFileExtension(trackUri);
    if (stringops::lowercase(extension) == "m3u")
    {
        upnp::HttpClient client(5);
        auto m3ufile = client.getText(trackUri);
        auto uris = audio::M3uParser::parseFileContents(m3ufile);
        
        std::lock_guard<std::recursive_mutex> lock(m_TracksMutex);
        for (auto& uri : uris)
        {
            m_Tracks.push_back(uri);
            log::info("Track queued: %s", uri);
        }
    }
    else
    {
        std::lock_guard<std::recursive_mutex> lock(m_TracksMutex);
        m_Tracks.push_back(trackUri);
        log::info("Track queued: %s", trackUri);
    }
}

void PlayQueue::clear()
{
    std::lock_guard<std::recursive_mutex> lock(m_TracksMutex);
    m_Tracks.clear();
    QueueChanged();
}

std::string PlayQueue::nextTrack() const
{
    std::lock_guard<std::recursive_mutex> lock(m_TracksMutex);
    if (m_Tracks.empty())
    {
        return "";
    }
    
    return m_Tracks.front();
}

bool PlayQueue::dequeueNextTrack(std::string& track)
{
    {
        std::lock_guard<std::recursive_mutex> lock(m_TracksMutex);
        if (m_Tracks.empty())
        {
            return false;
        }
        
        track = m_Tracks.front();
        m_Tracks.pop_front();
    }
    
    QueueChanged();
    
    return true;
}

size_t PlayQueue::getNumberOfTracks() const
{
    std::lock_guard<std::recursive_mutex> lock(m_TracksMutex);
    return m_Tracks.size();
}

}
