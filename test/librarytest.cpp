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
//│   ├── song\ 2.mp3
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
    for (auto& item : items)
        std::cout << item->getTitle() << std::endl;
    EXPECT_EQ(3, items.size());
}

}
}
