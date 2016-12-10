//    Copyright (C) 2012 Dirk Vanden Boer <dirk.vdb@gmail.com>
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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace utils;
using namespace testing;
using namespace std::string_literals;
using namespace std::chrono_literals;

namespace doozy
{
namespace test
{

class SignalMock
{
public:
    MOCK_METHOD0(QueueChanged, void());
    MOCK_METHOD1(CurrentTransportUriChanged, void(std::string));
    MOCK_METHOD1(NextTransportUriChanged, void(std::string));
};

class PlayQueueTest : public Test
{
protected:
    PlayQueueTest()
    : m_playQueue()
    {
        m_playQueue.QueueChanged.connect([this] () { m_mock.QueueChanged(); }, this);
        m_playQueue.CurrentTransportUriChanged.connect([this] (auto&& uri) { m_mock.CurrentTransportUriChanged(uri); }, this);
        m_playQueue.NextTransportUriChanged.connect([this] (auto&& uri) { m_mock.NextTransportUriChanged(uri); }, this);
    }

    PlayQueue m_playQueue;
    StrictMock<SignalMock> m_mock;
};

TEST_F(PlayQueueTest, SetCurrentUri)
{
    EXPECT_CALL(m_mock, QueueChanged());
    EXPECT_CALL(m_mock, CurrentTransportUriChanged("Uri"));

    m_playQueue.setCurrentUri("Uri");
}

TEST_F(PlayQueueTest, SetCurrentAndNextUriThenDeque)
{
    EXPECT_CALL(m_mock, QueueChanged()).Times(4);

    InSequence seq;
    EXPECT_CALL(m_mock, CurrentTransportUriChanged("Uri"));
    EXPECT_CALL(m_mock, NextTransportUriChanged("NextUri"));

    m_playQueue.setCurrentUri("Uri");
    m_playQueue.setNextUri("NextUri");

    auto track = m_playQueue.dequeueNextTrack();
    EXPECT_EQ("Uri", track->getUri());

    EXPECT_CALL(m_mock, CurrentTransportUriChanged("NextUri"));
    EXPECT_CALL(m_mock, NextTransportUriChanged(""));

    track = m_playQueue.dequeueNextTrack();
    EXPECT_EQ("NextUri", track->getUri());
}

}
}
