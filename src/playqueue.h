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
#include "audio/audioplaylistinterface.h"

#include <deque>
#include <string>
#include <mutex>

namespace doozy
{

class PlayQueue : public audio::IPlaylist
{
public:
    PlayQueue();
    virtual ~PlayQueue();

    void addTrack(const std::string& track);
    void clear();
    
    std::string currentTrack() const;
    std::string nextTrack() const;
    
    // IPlaylist
    virtual bool dequeueNextTrack(std::string& track);
    size_t getNumberOfTracks() const;
    
    utils::Signal<void()> QueueChanged;

private:
    std::deque<std::string>             m_Tracks;
    std::map<std::string, int32_t>		m_IndexMap;

    mutable std::recursive_mutex        m_TracksMutex;
    std::mutex                          m_SubscribersMutex;

    bool                                m_Destroy;
};

}

#endif
