#include <gtest/gtest.h>

#include <sqlite3.h>

#include "doozytestconfig.h"
#include "eventnotification.h"
#include "settingsmocks.h"

#include "serversettings.h"
#include "server/library/musiclibraryinterface.h"
#include "server/library/filesystemmusiclibrary.h"

#include "Utils/log.h"
#include "Utils/fileoperations.h"
#include "Utils/stringoperations.h"
#include "Utils/numericoperations.h"

#include "upnp/upnpitem.h"
#include "upnp/upnphttpclient.h"

#define TEST_DB "test.db"
//#define PERFORMANCE_TEST

using namespace std;
using namespace testing;

namespace doozy
{
namespace test
{

static const int64_t g_rootId = 0;
static const int64_t g_albumsId = 2;
static const int64_t g_browseFileSystemId = 3;

class LibraryTest : public testing::Test
{
    protected:

    virtual void SetUp()
    {
        std::vector<std::string> artFilenames = { "cover.jpg", "cover.png" };

        ON_CALL(m_settings, getDatabaseFilePath()).WillByDefault(Return(TEST_DB));
        ON_CALL(m_settings, getLibraryPath()).WillByDefault(Return(TEST_DATA_DIR));
        ON_CALL(m_settings, getAlbumArtFilenames()).WillByDefault(Return(artFilenames));

        m_library = std::make_unique<FilesystemMusicLibrary>(m_settings, "http://localhost:8080/Media/");
        m_library->OnScanComplete = [this] () {
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

    ServerSettingsMock              m_settings;
    std::unique_ptr<IMusicLibrary>  m_library;
    EventNotification               m_notification;
};

//├── audio
//│   ├── song & 2.mp3
//│   ├── song1.mp3
//│   └── subdir
//│       └── test.mp3
//├── delaytest.mp3
//└── delaytestwithid3.mp3

TEST_F(LibraryTest, GetRootContainer)
{
    auto item = m_library->getItem(std::to_string(g_rootId));
    EXPECT_EQ("0", item->getObjectId());
    EXPECT_EQ("-1", item->getParentId());
    EXPECT_EQ(upnp::Class::Container, item->getClass());
}

TEST_F(LibraryTest, GetItems)
{
    auto items = m_library->getItems(std::to_string(g_browseFileSystemId), 0, 0);
    ASSERT_EQ(3u, items.size());

    EXPECT_EQ("audio", items[0]->getTitle());
    EXPECT_EQ("delaytest.mp3", items[1]->getTitle());
    EXPECT_EQ("aTitle1", items[2]->getTitle());

    items = m_library->getItems(items[0]->getObjectId(), 0, 0);
    ASSERT_EQ(3u, items.size());
    EXPECT_EQ("subdir", items[0]->getTitle());
    EXPECT_EQ("aTitle3", items[1]->getTitle());
    EXPECT_EQ("aTitle2", items[2]->getTitle());
}

TEST_F(LibraryTest, GetAlbum)
{
    auto albums = m_library->getItems(std::to_string(g_albumsId), 0, 0);
    EXPECT_EQ(1u, albums.size());
    
    auto album = albums.front();
    EXPECT_EQ("anAlbum", album->getTitle());
    EXPECT_EQ("anAlbumArtist", album->getMetaData(upnp::Property::Artist));
    EXPECT_EQ(upnp::Class::AudioContainer, album->getClass());
}

TEST_F(LibraryTest, GetAlbumTracks)
{
    auto albums = m_library->getItems(std::to_string(g_albumsId), 0, 0);
    ASSERT_EQ(1u, albums.size());
    
    auto album = albums.front();
    auto songs = m_library->getItems(albums.front()->getObjectId(), 0, 0);
    EXPECT_EQ(4u, songs.size());
    
    for (auto& song : songs)
    {
        EXPECT_EQ(song->getParentId(), album->getObjectId());
        EXPECT_EQ(upnp::Class::Audio, song->getClass());
    }
}


//TEST_F(LibraryTest, DownloadFile)
//{
//    auto item = m_library->getItems(std::to_string(g_browseFileSystemId), 1, 1).front();
//    
//    auto res = item->getResources();
//    EXPECT_EQ(1, res.size());
//
//    upnp::HttpClient client(10);
//    auto data = client.getData(res.front().getUrl());
//    EXPECT_GT(0, data.size());
//}

#ifdef PERFORMANCE_TEST

TEST_F(LibraryTest, FullScan)
{
    m_settings.set("MusicLibrary", "/Volumes/Data/Music");

    FullScan();
}

#endif


}
}
