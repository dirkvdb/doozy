#include <gtest/gtest.h>

#include <sqlite3.h>

#include "server/library/musicdb.h"

#include "Utils/stringoperations.h"
#include "Utils/fileoperations.h"
#include "Utils/numericoperations.h"

#define TEST_DB "test.db"

using namespace std;
namespace doozy
{
namespace test
{

class LibraryDatabaseTest : public testing::Test
{
protected:
    virtual void SetUp()
    {
        try { utils::fileops::deleteFile(TEST_DB); } catch (...) {}
        m_db.reset(new MusicDb(TEST_DB));

        m_item.upnpItem = std::make_shared<upnp::Item>("#1", "TestItem");
        m_item.upnpItem->setParentId("0");
        m_item.upnpItem->setRefId("#88");
        m_item.upnpItem->setClass("object.container");
        m_item.modifiedTime = 100;
        m_item.path = "/the/path";
    }

    virtual void TearDown()
    {
        m_db.reset();
        utils::fileops::deleteFile(TEST_DB);
    }
    
    void addItem(const std::string& id, const std::string& parent, const std::string& title)
    {
        LibraryItem item;
        item.path = "/path";
        item.upnpItem = std::make_shared<upnp::Item>(id, "TestItem");
        item.upnpItem->setParentId(parent);
        item.upnpItem->setTitle(title);
        m_db->addItem(item);
    }

    std::unique_ptr<MusicDb>    m_db;
    LibraryItem                 m_item;
};

TEST_F(LibraryDatabaseTest, GetObjectCount)
{
    EXPECT_EQ(0, m_db->getObjectCount());
    m_db->addItem(m_item);
    EXPECT_EQ(1, m_db->getObjectCount());
}

TEST_F(LibraryDatabaseTest, GetChildCount)
{
    addItem("0", "-1", "root");
    addItem("0#1", "0", "item1");
    addItem("0#2", "0", "item2");

    EXPECT_EQ(2, m_db->getChildCount("0"));
    EXPECT_EQ(0, m_db->getChildCount("0#1"));
    EXPECT_EQ(0, m_db->getChildCount("0#2"));
}

TEST_F(LibraryDatabaseTest, ItemExists)
{
    std::string id;
    EXPECT_FALSE(m_db->itemExists(m_item.path, id));
    m_db->addItem(m_item);
    EXPECT_TRUE(m_db->itemExists(m_item.path, id));
    EXPECT_EQ(m_item.upnpItem->getObjectId(), id);
}

TEST_F(LibraryDatabaseTest, ItemStatus)
{
    m_db->addItem(m_item);
    EXPECT_EQ(ItemStatus::DoesntExist, m_db->getItemStatus("/bad/path", 100));
    EXPECT_EQ(ItemStatus::UpToDate, m_db->getItemStatus(m_item.path, m_item.modifiedTime));
    EXPECT_EQ(ItemStatus::NeedsUpdate, m_db->getItemStatus(m_item.path, m_item.modifiedTime + 1));
}

TEST_F(LibraryDatabaseTest, AddGetItem)
{
    m_db->addItem(m_item);
    auto item = m_db->getItem(m_item.upnpItem->getObjectId());

    EXPECT_EQ(m_item.upnpItem->getObjectId(),   item.upnpItem->getObjectId());
    EXPECT_EQ(m_item.upnpItem->getRefId(),      item.upnpItem->getRefId());
    EXPECT_EQ(m_item.upnpItem->getParentId(),   item.upnpItem->getParentId());
    EXPECT_EQ(m_item.upnpItem->getTitle(),      item.upnpItem->getTitle());
    EXPECT_EQ(m_item.upnpItem->getClass(),      item.upnpItem->getClass());
}

TEST_F(LibraryDatabaseTest, AddGetItemAmpersand)
{
    m_item.upnpItem->setTitle("Me & my title");

    m_db->addItem(m_item);
    auto item = m_db->getItem(m_item.upnpItem->getObjectId());

    EXPECT_EQ(m_item.upnpItem->getObjectId(),   item.upnpItem->getObjectId());
    EXPECT_EQ(m_item.upnpItem->getRefId(),      item.upnpItem->getRefId());
    EXPECT_EQ(m_item.upnpItem->getParentId(),   item.upnpItem->getParentId());
    EXPECT_EQ(m_item.upnpItem->getTitle(),      item.upnpItem->getTitle());
    EXPECT_EQ(m_item.upnpItem->getClass(),      item.upnpItem->getClass());
}

TEST_F(LibraryDatabaseTest, AddGetItemLongPath)
{
    //m_item.upnpItem->setTitle("02 - 14 - David Bowie - Dancing In The Street (ft. Mick Jagger).mp3");
    m_item.upnpItem->setTitle("----------------------------------------------------------------");

    m_db->addItem(m_item);
    auto item = m_db->getItem(m_item.upnpItem->getObjectId());

    EXPECT_EQ(m_item.upnpItem->getObjectId(),   item.upnpItem->getObjectId());
    EXPECT_EQ(m_item.upnpItem->getRefId(),      item.upnpItem->getRefId());
    EXPECT_EQ(m_item.upnpItem->getParentId(),   item.upnpItem->getParentId());
    EXPECT_EQ(m_item.upnpItem->getTitle(),      item.upnpItem->getTitle());
    EXPECT_EQ(m_item.upnpItem->getClass(),      item.upnpItem->getClass());
}

TEST_F(LibraryDatabaseTest, UpdateItem)
{
    m_db->addItem(m_item);

    m_item.upnpItem->setParentId("NewParent");
    m_item.upnpItem->setRefId("NewRefId");
    m_item.upnpItem->setTitle("NewTitle");
    m_item.upnpItem->setClass("object.audiocontainer");
    m_item.modifiedTime = 200;

    m_db->updateItem(m_item);
    auto item = m_db->getItem(m_item.upnpItem->getObjectId());

    EXPECT_EQ(m_item.upnpItem->getObjectId(),   item.upnpItem->getObjectId());
    EXPECT_EQ(m_item.upnpItem->getRefId(),      item.upnpItem->getRefId());
    EXPECT_EQ(m_item.upnpItem->getParentId(),   item.upnpItem->getParentId());
    EXPECT_EQ(m_item.upnpItem->getTitle(),      item.upnpItem->getTitle());
    EXPECT_EQ(m_item.upnpItem->getClass(),      item.upnpItem->getClass());
    EXPECT_EQ(m_item.modifiedTime,              item.modifiedTime);
    EXPECT_EQ(m_item.path,                      item.path);
}

TEST_F(LibraryDatabaseTest, GetItems)
{
    addItem("0", "-1", "root");
    addItem("0#1", "0", "item1");
    addItem("0#2", "0", "item2");
    addItem("0#3", "0", "item3");
    
    auto items = m_db->getItems("0", 0, 0);
    EXPECT_EQ(3, items.size());
}

TEST_F(LibraryDatabaseTest, GetItemsOffsetCount)
{
    addItem("0", "-1", "root");
    addItem("0#1", "0", "item1");
    addItem("0#2", "0", "item2");
    addItem("0#3", "0", "item3");
    
    // 1 item beginning at offset 1
    auto items = m_db->getItems("0", 1, 1);
    EXPECT_EQ(1, items.size());
    
    // 2 item beginning at offset 1
    items = m_db->getItems("0", 1, 2);
    EXPECT_EQ(2, items.size());
    
    // 3 item beginning at offset 1 -> results in 2 items
    items = m_db->getItems("0", 1, 3);
    EXPECT_EQ(2, items.size());
    
    // all items beginning at offset 1 -> results in 2 items
    items = m_db->getItems("0", 1, 0);
    EXPECT_EQ(2, items.size());
    
    // all items beginning at offset 3 -> results in 0 items
    items = m_db->getItems("0", 3, 0);
    EXPECT_EQ(0, items.size());
}

//TEST_F(LibraryDatabaseTest, TrackExists)
//{
//    pDb->addTrack(track);
//
//    EXPECT_TRUE(pDb->trackExists(track.filepath));
//    EXPECT_FALSE(pDb->trackExists("/some/path/track.mp3"));
//}
//
//TEST_F(LibraryDatabaseTest, GetTrackStatus)
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
//TEST_F(LibraryDatabaseTest, AddTrack)
//{
//    Track returnedTrack;
//    pDb->addTrack(track);
//    ASSERT_TRUE(pDb->getTrackWithPath(track.filepath, returnedTrack));
//    EXPECT_EQ(track, returnedTrack);
//}
//
//TEST_F(LibraryDatabaseTest, AddTwoTracks)
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
//TEST_F(LibraryDatabaseTest, AddIncompleteLibraryTrack)
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
//TEST_F(LibraryDatabaseTest, TrackCount)
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
//TEST_F(LibraryDatabaseTest, RemoveNonExistingTracks)
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
//TEST_F(LibraryDatabaseTest, GetAlbums)
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
//TEST_F(LibraryDatabaseTest, GetFirstSongFromAlbum)
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
//TEST_F(LibraryDatabaseTest, GetSongsFromAlbum)
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
//TEST_F(LibraryDatabaseTest, setAlbumArt)
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
