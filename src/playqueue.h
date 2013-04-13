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

#ifndef DOOZY_PLAYQUEUE_H
#define DOOZY_PLAYQUEUE_H

#include "utils/subscriber.h"
#include "utils/types.h"
#include "utils/signal.h"

#include "audio/audiotrackinterface.h"
#include "audio/audioplaylistinterface.h"

#include <deque>
#include <string>
#include <mutex>

namespace doozy
{

class PlayQueueItem : public audio::ITrack
{
public:
    PlayQueueItem() = default;
    PlayQueueItem(const std::string& avTransportUri);
    PlayQueueItem(const std::string& trackUri, const std::string& avTransportUri);

    // ITrack
    virtual std::string getUri() const;
    
    std::string getAVTransportUri() const;
    
private:
    std::string m_TrackUri;
    std::string m_AVTransportUri;
};

typedef std::shared_ptr<PlayQueueItem> PlayQueueItemPtr;

class PlayQueue : public audio::IPlaylist
{
public:
    PlayQueue();
    virtual ~PlayQueue();

    void setCurrentUri(const std::string& avTransportUri);
    void setNextUri(const std::string& avTransportUri);
    std::string getCurrentUri() const;
    std::string getNextUri() const;
    
    void clear();
    
    // IPlaylist
    virtual std::shared_ptr<audio::ITrack> dequeueNextTrack();
    size_t getNumberOfTracks() const;
    
    utils::Signal<void()> QueueChanged;
    utils::Signal<void(std::string)> CurrentTransportUriChanged;
    utils::Signal<void(std::string)> NextTransportUriChanged;

private:
    std::deque<PlayQueueItemPtr>        m_CurrenURITracks;
    std::deque<PlayQueueItemPtr>        m_NextURITracks;
    std::map<std::string, int32_t>		m_IndexMap;

    mutable std::recursive_mutex        m_TracksMutex;
    std::mutex                          m_SubscribersMutex;

    bool                                m_Destroy;
};

}

#endif
