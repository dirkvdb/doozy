#include <gtest/gtest.h>

#include <sqlite3.h>

#include "doozytestconfig.h"
#include "eventnotification.h"

#include "common/settings.h"
#include "server/library/musiclibraryinterface.h"
#include "server/library/musiclibraryfactory.h"

#include "Utils/fileoperations.h"
#include "Utils/stringoperations.h"
#include "Utils/numericoperations.h"

#define TEST_DB "test.db"

using namespace std;
namespace doozy
{
namespace test
{

class LibraryTest : public testing::Test
{
    protected:

    virtual void SetUp()
    {
        m_settings.set("MusicLibrary", std::string(TEST_DATA_DIR));
        m_settings.set("DBFile", std::string(TEST_DB));

        m_library.reset(MusicLibraryFactory::create(MusicLibraryType::FileSystem, m_settings));
        m_library->OnScanComplete = [this] {
            m_notification.triggerEvent();
        };
        
        FullScan();
    }

    virtual void TearDown()
    {
        m_library.reset();
        utils::fileops::deleteFile(TEST_DB);
    }
    
    void FullScan()
    {
        m_library->scan(true);
        EXPECT_TRUE(m_notification.waitForEvent());
    }

    Settings                        m_settings;
    std::unique_ptr<IMusicLibrary>  m_library;
    EventNotification               m_notification;
};

TEST_F(LibraryTest, GetRootContainer)
{
    auto item = m_library->getItem("0");
    EXPECT_EQ("0", item->getObjectId());
    EXPECT_EQ("-1", item->getParentId());
    EXPECT_EQ(upnp::Item::Class::Container, item->getClass());
}

//TEST_F(LibraryTest, TrackExists)
//{
//    pDb->addTrack(track);
//
//    EXPECT_TRUE(pDb->trackExists(track.filepath));
//    EXPECT_FALSE(pDb->trackExists("/some/path/track.mp3"));
//}
//
//TEST_F(LibraryTest, GetTrackStatus)
//{
//    pDb->addTrack(track);
//
//    EXPECT_EQ(MusicDb::DoesntExist, pDb->getTrackStatus("/new/path", 1234));
//    EXPECT_EQ(MusicDb::NeedsUpdate, pDb->getTrackStatus(track.filepath, 10001));
//    EXPECT_EQ(MusicDb::UpToDate, pDb->getTrackStatus(track.filepath, 10000));
//    EXPECT_EQ(MusicDb::UpToDate, pDb->getTrackStatus(track.filepath, 9999));
//}
//
//
//TEST_F(LibraryTest, AddTrack)
//{
//    Track returnedTrack;
//    pDb->addTrack(track);
//    ASSERT_TRUE(pDb->getTrackWithPath(track.filepath, returnedTrack));
//    EXPECT_EQ(track, returnedTrack);
//}
//
//TEST_F(LibraryTest, AddTwoTracks)
//{
//    Track returnedTrack;
//
//    pDb->addTrack(track);
//    track.filepath = "anotherPath";
//    track.title = "anotherTitle";
//    pDb->addTrack(track);
//    ASSERT_TRUE(pDb->getTrackWithPath(track.filepath, returnedTrack));
//    track.id = "2";
//    EXPECT_EQ(track, returnedTrack);
//}
//
//TEST_F(LibraryTest, AddIncompleteLibraryTrack)
//{
//    Track returnedTrack;
//
//    track.album = "Unknown Album";
//    track.artist = "Unknown Artist";
//    track.albumArtist = "";
//    track.genre = "";
//
//    Album anAlbum;
//    anAlbum.title = track.album;
//    anAlbum.artist = track.albumArtist;
//    pDb->addAlbum(anAlbum);
//    pDb->addTrack(track);
//    ASSERT_TRUE(pDb->getTrackWithPath(track.filepath, returnedTrack));
//    track.albumId = anAlbum.id;
//    EXPECT_EQ(track, returnedTrack);
//}
//
//TEST_F(LibraryTest, TrackCount)
//{
//    EXPECT_EQ(0, pDb->getTrackCount());
//
//    pDb->addTrack(track);
//    EXPECT_EQ(1, pDb->getTrackCount());
//
//    track.filepath = "otherpath";
//    pDb->addTrack(track);
//    EXPECT_EQ(2, pDb->getTrackCount());
//}
//
//TEST_F(LibraryTest, RemoveNonExistingTracks)
//{
//    pDb->addTrack(track); //non existing path
//    track.filepath = TEST_DB;
//    track.id = "otherid";
//    pDb->addTrack(track); //existing path
//    EXPECT_EQ(2, pDb->getTrackCount());
//
//    pDb->removeNonExistingFiles();
//    EXPECT_EQ(1, pDb->getTrackCount());
//    EXPECT_EQ(1, subscriber.deletedTracks.size());
//    EXPECT_EQ("1", subscriber.deletedTracks[0]);
//}
//
//TEST_F(LibraryTest, GetAlbums)
//{
//    AlbumSubscriberMock albumSubscriber;
//    Album anAlbum;
//
//    pDb->addTrack(track);
//    track.album = "anotherAlbum";
//    track.filepath = "anotherPath";
//    anAlbum.title = track.album;
//    anAlbum.artist = track.albumArtist;
//    pDb->addAlbum(anAlbum);
//    pDb->addTrack(track);
//
//    pDb->getAlbums(albumSubscriber);
//    ASSERT_EQ(2, albumSubscriber.albums.size());
//
//    Album album;
//    album.artist = "anAlbumArtist";
//    album.title = "anAlbum";
//    EXPECT_EQ(album, albumSubscriber.albums[0]);
//
//    album.title = "anotherAlbum";
//    EXPECT_EQ(album, albumSubscriber.albums[1]);
//}
//
//TEST_F(LibraryTest, GetFirstSongFromAlbum)
//{
//    TrackSubscriberMock trackSubscriber;
//
//    pDb->addTrack(track);
//
//    Track otherTrack = track;
//    otherTrack.filepath = "anotherPath";
//    otherTrack.title = "anotherTitle";
//    otherTrack.id++;
//    pDb->addTrack(otherTrack);
//
//    pDb->getFirstTrackFromAlbum(track.albumId, trackSubscriber);
//    ASSERT_EQ(1, trackSubscriber.tracks.size());
//    EXPECT_EQ(track, trackSubscriber.tracks[0]);
//}
//
//TEST_F(LibraryTest, GetSongsFromAlbum)
//{
//    TrackSubscriberMock trackSubscriber;
//
//    pDb->addTrack(track);
//
//    Track otherTrack = track;
//    otherTrack.filepath = "anotherPath";
//    otherTrack.title = "anotherTitle";
//    
//    uint32_t id;
//    StringOperations::toNumeric(track.id, id);
//    NumericOperations::toString(++id, otherTrack.id);
//    pDb->addTrack(otherTrack);    
//
//    pDb->getTracksFromAlbum(track.albumId, trackSubscriber);
//    ASSERT_EQ(2, trackSubscriber.tracks.size());
//    EXPECT_EQ(track, trackSubscriber.tracks[0]);
//    EXPECT_EQ(otherTrack, trackSubscriber.tracks[1]);
//}
//
//TEST_F(LibraryTest, setAlbumArt)
//{
//    std::vector<uint8_t> data(8, 6);
//    
//    pDb->setAlbumArt(album.id, data);
//
//    pDb->getAlbumArt(album);
//
//    ASSERT_EQ(data.size(), album.coverData.size());
//    EXPECT_EQ(0, memcmp(&data.front(), &album.coverData.front(), 8));
//}

}
}
