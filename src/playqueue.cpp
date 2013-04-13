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

PlayQueueItem::PlayQueueItem(const std::string& avTransportUri)
: m_TrackUri(avTransportUri)
, m_AVTransportUri(avTransportUri)
{
}

PlayQueueItem::PlayQueueItem(const std::string& trackUri, const std::string& avTransportUri)
: m_TrackUri(trackUri)
, m_AVTransportUri(avTransportUri)
{
}

std::string PlayQueueItem::getUri() const
{
    return m_TrackUri;
}

std::string PlayQueueItem::getAVTransportUri() const
{
    return m_AVTransportUri;
}

PlayQueue::PlayQueue()
: m_Destroy(false)
{
}

PlayQueue::~PlayQueue()
{
    m_Destroy = true;
}

static std::deque<PlayQueueItemPtr> getTracksFromUri(const std::string& transportUri)
{
    std::deque<PlayQueueItemPtr> items;

    const std::string extension = fileops::getFileExtension(transportUri);
    if (stringops::lowercase(extension) == "m3u")
    {
        upnp::HttpClient client(5);
        auto m3ufile = client.getText(transportUri);
        auto uris = audio::M3uParser::parseFileContents(m3ufile);
        
        for (auto& uri : uris)
        {
            items.push_back(std::make_shared<PlayQueueItem>(uri, transportUri));
        }
    }
    else
    {
        items.push_back(std::make_shared<PlayQueueItem>(transportUri));
    }
    
    return items;
}

void PlayQueue::setCurrentUri(const std::string& avTransportUri)
{
    {
        std::lock_guard<std::recursive_mutex> lock(m_TracksMutex);
        m_CurrenURITracks = getTracksFromUri(avTransportUri);
    }
    
    CurrentTransportUriChanged(avTransportUri);
    QueueChanged();
}

void PlayQueue::setNextUri(const std::string& avTransportUri)
{
    {
        std::lock_guard<std::recursive_mutex> lock(m_TracksMutex);
        m_NextURITracks = getTracksFromUri(avTransportUri);
    }
    
    NextTransportUriChanged(avTransportUri);
    QueueChanged();
}

std::string PlayQueue::getCurrentUri() const
{
    std::lock_guard<std::recursive_mutex> lock(m_TracksMutex);
    return m_CurrenURITracks.empty() ? "" : m_CurrenURITracks.front()->getAVTransportUri();
}

std::string PlayQueue::getNextUri() const
{
    std::lock_guard<std::recursive_mutex> lock(m_TracksMutex);
    return m_NextURITracks.empty() ? "" : m_NextURITracks.front()->getAVTransportUri();
}

void PlayQueue::clear()
{
    {
        std::lock_guard<std::recursive_mutex> lock(m_TracksMutex);
        m_CurrenURITracks.clear();
        m_NextURITracks.clear();
    }
    
    QueueChanged();
}

std::shared_ptr<audio::ITrack> PlayQueue::dequeueNextTrack()
{
    bool avTransportUriChange = false;
    PlayQueueItemPtr track;

    {
        std::lock_guard<std::recursive_mutex> lock(m_TracksMutex);
        if (m_CurrenURITracks.empty() && m_NextURITracks.empty())
        {
            return track;
        }
        else if (m_CurrenURITracks.empty() && !m_NextURITracks.empty())
        {
            std::swap(m_CurrenURITracks, m_NextURITracks);
            avTransportUriChange = true;
        }
        
        if (!m_CurrenURITracks.empty())
        {
            track = m_CurrenURITracks.front();
            m_CurrenURITracks.pop_front();
        }
    }
    
    if (avTransportUriChange)
    {
        CurrentTransportUriChanged(m_CurrenURITracks.front()->getAVTransportUri());
        NextTransportUriChanged("");
    }
    
    QueueChanged();
    
    return track;
}

size_t PlayQueue::getNumberOfTracks() const
{
    std::lock_guard<std::recursive_mutex> lock(m_TracksMutex);
    return m_CurrenURITracks.size();
}

}
