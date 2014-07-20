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
//#define PERFORMANCE_TEST

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
        m_settings.set("MusicLibrary", TEST_DATA_DIR);
        m_settings.set("DBFile", TEST_DB);

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

//├── audio
//│   ├── song & 2.mp3
//│   ├── song1.mp3
//│   └── subdir
//│       └── test.mp3
//├── delaytest.mp3
//└── delaytestwithid3.mp3

TEST_F(LibraryTest, GetRootContainer)
{
    auto item = m_library->getItem("0");
    EXPECT_EQ("0", item->getObjectId());
    EXPECT_EQ("-1", item->getParentId());
    EXPECT_EQ(upnp::Item::Class::Container, item->getClass());
}

TEST_F(LibraryTest, GetItems)
{
    auto items = m_library->getItems("#1", 0, 0);
    ASSERT_EQ(3, items.size());

    EXPECT_EQ("audio", items[0]->getTitle());
    EXPECT_EQ("delaytest.mp3", items[1]->getTitle());
    EXPECT_EQ("delaytestwithid3.mp3", items[2]->getTitle());

    items = m_library->getItems("#1#1", 0, 0);
    ASSERT_EQ(3, items.size());
    EXPECT_EQ("subdir", items[2]->getTitle());
    EXPECT_EQ("song1.mp3", items[1]->getTitle());
    EXPECT_EQ("song & 2.mp3", items[0]->getTitle());
}

#ifdef PERFORMANCE_TEST

TEST_F(LibraryTest, FullScan)
{
    m_settings.set("MusicLibrary", "/Volumes/Data/Music");

    FullScan();
}

#endif


}
}
