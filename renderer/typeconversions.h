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


#pragma once

#include <sstream>

#include "audio/audioplaybackinterface.h"

namespace doozy
{

inline std::ostream& operator<< (std::ostream& os, const audio::PlaybackState& state)
{
    switch (state)
    {
        case audio::PlaybackState::Stopped:      return os << "Stopped";
        case audio::PlaybackState::Playing:      return os << "Playing";
        case audio::PlaybackState::Paused:       return os << "Paused";
    }

    return os;
}

inline std::ostream& operator<< (std::ostream& os, const audio::PlaybackAction& action)
{
    switch (action)
    {
        case audio::PlaybackAction::Play:        return os << "Play";
        case audio::PlaybackAction::Pause:       return os << "Pause";
        case audio::PlaybackAction::Stop:        return os << "Stop";
        case audio::PlaybackAction::Prev:        return os << "Previous";
        case audio::PlaybackAction::Next:        return os << "Next";
    }

    return os;
}

template <typename T>
inline std::string toString(const T& t)
{
    std::stringstream ss;
    ss << t;

    return ss.str();
}

}
