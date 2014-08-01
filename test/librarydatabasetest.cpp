#include <gtest/gtest.h>

#include <sqlite3.h>

#include "server/library/musicdb.h"

#include "Utils/stringoperations.h"
#include "Utils/fileoperations.h"
#include "Utils/numericoperations.h"

#include "upnp/upnpitem.h"

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
        m_db->setWebRoot("http://localhost:8080/Media/");

        m_item.objectId = "#1";
        m_item.title = "TestItem";
        m_item.parentId = "0";
        m_item.refId = "#88";
        m_item.upnpClass = "object.container";
        m_item.modifiedTime = 100;
        m_item.path = "/the/path";
        m_item.name = "path";
    }

    virtual void TearDown()
    {
        m_db.reset();
        utils::fileops::deleteFile(TEST_DB);
    }
    
    void addItem(const std::string& id, const std::string& parent, const std::string& title)
    {
        m_db->addItem(createItem(id, parent, title));
    }
    
    LibraryItem createItem(const std::string& id, const std::string& parent, const std::string& title)
    {
        LibraryItem item;
        item.path = "/the/path_" + title;
        item.name = "path";
        item.objectId = id;
        item.title = title;
        item.parentId = parent;
        item.upnpClass = "object.container";

        return item;
    }
    
    LibraryItem createAlbum(const std::string& index, const std::string& title, const std::string& artist)
    {
        LibraryItem item;
        item.path = "/the/path_" + title;
        item.name = title;
        item.objectId = "0@1@1" + index;
        item.title = title;
        item.artist = artist;
        item.parentId = "0@1@1";
        item.upnpClass = "object.container.album.musicAlbum";

        return item;
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
    EXPECT_EQ(m_item.objectId, id);
}

TEST_F(LibraryDatabaseTest, AlbumExists)
{
    const std::string title = "MyTitle";
    const std::string artist = "MyArtist";
    
    std::string albumId;
    EXPECT_FALSE(m_db->albumExists(title, artist, albumId));

    auto album = createAlbum("1", title, artist);
    m_db->addItem(album);

    EXPECT_TRUE(m_db->albumExists(title, artist, albumId));
    EXPECT_EQ(album.objectId, albumId);
}

TEST_F(LibraryDatabaseTest, AlbumExistsNoArtist)
{
    const std::string title = "MyTitle";
    const std::string artist;
    
    std::string albumId;
    EXPECT_FALSE(m_db->albumExists(title, artist, albumId));

    auto album = createAlbum("1", title, artist);
    m_db->addItem(album);

    EXPECT_TRUE(m_db->albumExists(title, artist, albumId));
    EXPECT_EQ(album.objectId, albumId);
}


TEST_F(LibraryDatabaseTest, ItemStatus)
{
    m_db->addItem(m_item);
    EXPECT_EQ(ItemStatus::DoesntExist, m_db->getItemStatus("/bad/path", 100));
    EXPECT_EQ(ItemStatus::UpToDate, m_db->getItemStatus(m_item.path, m_item.modifiedTime));
    EXPECT_EQ(ItemStatus::NeedsUpdate, m_db->getItemStatus(m_item.path, m_item.modifiedTime + 1));
}

TEST_F(LibraryDatabaseTest, GetItemPath)
{
    m_db->addItem(m_item);
    EXPECT_EQ(m_item.path, m_db->getItemPath(m_item.objectId));
}

TEST_F(LibraryDatabaseTest, AddGetItem)
{
    m_db->addItem(m_item);
    auto item = m_db->getItem(m_item.objectId);

    EXPECT_EQ(m_item.objectId,      item->getObjectId());
    EXPECT_EQ(m_item.refId,         item->getRefId());
    EXPECT_EQ(m_item.parentId,      item->getParentId());
    EXPECT_EQ(m_item.title,         item->getTitle());
    EXPECT_EQ(m_item.upnpClass,     item->getClassString());
}

TEST_F(LibraryDatabaseTest, AddItems)
{
    std::vector<LibraryItem> items = {
        createItem("0", "-1", "root"),
        createItem("0#1", "0", "item1"),
        createItem("0#2", "0", "item2")
    };

    m_db->addItems(items);
    EXPECT_EQ(3, m_db->getObjectCount());
    
    auto item = m_db->getItem("0");
    EXPECT_EQ("root", item->getTitle());
}

TEST_F(LibraryDatabaseTest, AddGetItemAmpersand)
{
    m_item.title = "Me & my title";

    m_db->addItem(m_item);
    auto item = m_db->getItem(m_item.objectId);

    EXPECT_EQ(m_item.objectId,   item->getObjectId());
    EXPECT_EQ(m_item.refId,      item->getRefId());
    EXPECT_EQ(m_item.parentId,   item->getParentId());
    EXPECT_EQ(m_item.title,      item->getTitle());
    EXPECT_EQ(m_item.upnpClass,  item->getClassString());
}

TEST_F(LibraryDatabaseTest, AddGetItemLongPath)
{
    m_item.title = "----------------------------------------------------------------";

    m_db->addItem(m_item);
    auto item = m_db->getItem(m_item.objectId);

    EXPECT_EQ(m_item.objectId,   item->getObjectId());
    EXPECT_EQ(m_item.refId,      item->getRefId());
    EXPECT_EQ(m_item.parentId,   item->getParentId());
    EXPECT_EQ(m_item.title,      item->getTitle());
    EXPECT_EQ(m_item.upnpClass,  item->getClassString());
}

TEST_F(LibraryDatabaseTest, UpdateItem)
{
    m_db->addItem(m_item);

    m_item.parentId     = "NewParent";
    m_item.refId        = "NewRefId";
    m_item.title        = "NewTitle";
    m_item.upnpClass    = "object.container.album.musicAlbum";
    m_item.modifiedTime = 200;

    m_db->updateItem(m_item);
    auto item = m_db->getItem(m_item.objectId);

    EXPECT_EQ(m_item.objectId,   item->getObjectId());
    EXPECT_EQ(m_item.refId,      item->getRefId());
    EXPECT_EQ(m_item.parentId,   item->getParentId());
    EXPECT_EQ(m_item.title,      item->getTitle());
    EXPECT_EQ(m_item.upnpClass,  item->getClassString());
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

}
}
