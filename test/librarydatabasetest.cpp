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

static const int64_t g_albumsId = 2;

class LibraryDatabaseTest : public testing::Test
{
protected:
    virtual void SetUp()
    {
        try { utils::fileops::deleteFile(TEST_DB); } catch (...) {}
        m_db = std::make_unique<MusicDb>(TEST_DB);
        m_db->setWebRoot("http://localhost:8080/Media/");

        m_item.objectId = 1;
        m_item.parentId = 0;
        m_item.refId = 8;
        m_item.upnpClass = "object.container";
        m_item.name = "path";
        m_meta.title = "TestItem";
        m_meta.modifiedTime = 100;
        m_meta.path = "/the/path";
    }

    virtual void TearDown()
    {
        m_db.reset();
        utils::fileops::deleteFile(TEST_DB);
    }
    
    void addItem(int64_t parent, const std::string& title)
    {
        auto item = createItem(0, parent, title);
        m_db->addItem(item.first, item.second);
    }
    
    void addItem(int64_t id, int64_t parent, const std::string& title)
    {
        auto item = createItem(id, parent, title);
        m_db->addItemWithId(item.first, item.second);
    }
    
    std::pair<LibraryItem, LibraryMetadata> createItem(int64_t id, int64_t parent, const std::string& title)
    {
        LibraryItem item;
        item.name = "path";
        item.objectId = id;
        item.parentId = parent;
        item.upnpClass = "object.container";

        LibraryMetadata meta;
        meta.path = "/the/path_" + title;
        meta.title = title;

        return {item, meta};
    }
    
    std::pair<LibraryItem, LibraryMetadata> createAlbum(int64_t id, const std::string& title, const std::string& artist)
    {
        LibraryItem item;
        item.name = title;
        item.objectId = id;
        item.parentId = g_albumsId;
        item.upnpClass = "object.container.album.musicAlbum";

        LibraryMetadata meta;
        meta.path = "/the/path_" + title;
        meta.title = title;
        meta.artist = artist;

        return {item, meta};
    }

    std::unique_ptr<MusicDb>    m_db;
    LibraryItem                 m_item;
    LibraryMetadata             m_meta;
};

TEST_F(LibraryDatabaseTest, GetObjectCount)
{
    EXPECT_EQ(0u, m_db->getObjectCount());
    m_db->addItem(m_item, m_meta);
    EXPECT_EQ(1u, m_db->getObjectCount());
}

TEST_F(LibraryDatabaseTest, GetChildCount)
{
    addItem(0, -1, "root");
    addItem(0, "item1");
    addItem(0, "item2");

    EXPECT_EQ(2u, m_db->getChildCount(0));
    EXPECT_EQ(0u, m_db->getChildCount(1));
    EXPECT_EQ(0u, m_db->getChildCount(2));
}

TEST_F(LibraryDatabaseTest, ItemExists)
{
    int64_t id;
    EXPECT_FALSE(m_db->itemExists(m_meta.path, id));
    m_db->addItem(m_item, m_meta);
    EXPECT_TRUE(m_db->itemExists(m_meta.path, id));
    EXPECT_EQ(m_item.objectId, id);
}

TEST_F(LibraryDatabaseTest, AlbumExists)
{
    const std::string title = "MyTitle";
    const std::string artist = "MyArtist";
    
    int64_t albumId;
    EXPECT_FALSE(m_db->albumExists(title, artist, albumId));

    auto album = createAlbum(1, title, artist);
    m_db->addItem(album.first, album.second);

    EXPECT_TRUE(m_db->albumExists(title, artist, albumId));
    EXPECT_EQ(album.first.objectId, albumId);
}

TEST_F(LibraryDatabaseTest, AlbumExistsNoArtist)
{
    const std::string title = "MyTitle";
    const std::string artist;
    
    int64_t albumId;
    EXPECT_FALSE(m_db->albumExists(title, artist, albumId));

    auto album = createAlbum(1, title, artist);
    m_db->addItem(album.first, album.second);

    EXPECT_TRUE(m_db->albumExists(title, artist, albumId));
    EXPECT_EQ(album.first.objectId, albumId);
}


TEST_F(LibraryDatabaseTest, ItemStatus)
{
    m_db->addItem(m_item, m_meta);
    EXPECT_EQ(ItemStatus::DoesntExist, m_db->getItemStatus("/bad/path", 100));
    EXPECT_EQ(ItemStatus::UpToDate, m_db->getItemStatus(m_meta.path, m_meta.modifiedTime));
    EXPECT_EQ(ItemStatus::NeedsUpdate, m_db->getItemStatus(m_meta.path, m_meta.modifiedTime + 1));
}

TEST_F(LibraryDatabaseTest, GetItemPath)
{
    m_db->addItem(m_item, m_meta);
    EXPECT_EQ(m_meta.path, m_db->getItemPath(m_item.objectId));
}

TEST_F(LibraryDatabaseTest, AddGetItem)
{
    m_db->addItem(m_item, m_meta);
    auto item = m_db->getItem(m_item.objectId);

    EXPECT_EQ(m_item.objectId,      std::stoll(item->getObjectId()));
    EXPECT_EQ(m_item.refId,         std::stoll(item->getRefId()));
    EXPECT_EQ(m_item.parentId,      std::stoll(item->getParentId()));
    EXPECT_EQ(m_item.upnpClass,     item->getClassString());
    EXPECT_EQ(m_meta.title,         item->getTitle());
}

TEST_F(LibraryDatabaseTest, AddItems)
{
    addItem(0, -1, "root");
    std::vector<std::pair<LibraryItem, LibraryMetadata>> items = {
        createItem(0, 0, "item1"),
        createItem(0, 0, "item2")
    };

    m_db->addItems(items);
    EXPECT_EQ(3u, m_db->getObjectCount());
    
    auto item = m_db->getItem(0);
    EXPECT_EQ("root", item->getTitle());
    
    item = m_db->getItem(1);
    EXPECT_EQ("item1", item->getTitle());
    EXPECT_EQ("0", item->getParentId());
    EXPECT_EQ("", item->getRefId());
    
    item = m_db->getItem(2);
    EXPECT_EQ("item2", item->getTitle());
    EXPECT_EQ("0", item->getParentId());
    EXPECT_EQ("", item->getRefId());
}

TEST_F(LibraryDatabaseTest, AddGetItemAmpersand)
{
    m_meta.title = "Me & my title";

    m_db->addItem(m_item, m_meta);
    auto item = m_db->getItem(m_item.objectId);

    EXPECT_EQ(m_item.objectId,   std::stoll(item->getObjectId()));
    EXPECT_EQ(m_item.refId,      std::stoll(item->getRefId()));
    EXPECT_EQ(m_item.parentId,   std::stoll(item->getParentId()));
    EXPECT_EQ(m_item.upnpClass,  item->getClassString());
    EXPECT_EQ(m_meta.title,      item->getTitle());
}

TEST_F(LibraryDatabaseTest, AddGetItemLongPath)
{
    m_meta.title = "----------------------------------------------------------------";

    m_db->addItem(m_item, m_meta);
    auto item = m_db->getItem(m_item.objectId);

    EXPECT_EQ(m_item.objectId,   std::stoll(item->getObjectId()));
    EXPECT_EQ(m_item.refId,      std::stoll(item->getRefId()));
    EXPECT_EQ(m_item.parentId,   std::stoll(item->getParentId()));
    EXPECT_EQ(m_item.upnpClass,  item->getClassString());
    EXPECT_EQ(m_meta.title,      item->getTitle());
}

TEST_F(LibraryDatabaseTest, UpdateItem)
{
    m_db->addItem(m_item, m_meta);

    m_item.parentId     = 4;
    m_item.refId        = 5;
    m_item.upnpClass    = "object.container.album.musicAlbum";
    m_meta.title        = "NewTitle";
    m_meta.modifiedTime = 200;

    m_db->updateItem(m_item, m_meta);
    auto item = m_db->getItem(m_item.objectId);

    EXPECT_EQ(m_item.objectId,   std::stoll(item->getObjectId()));
    EXPECT_EQ(m_item.refId,      std::stoll(item->getRefId()));
    EXPECT_EQ(m_item.parentId,   std::stoll(item->getParentId()));
    EXPECT_EQ(m_item.upnpClass,  item->getClassString());
    EXPECT_EQ(m_meta.title,      item->getTitle());
}

TEST_F(LibraryDatabaseTest, GetItems)
{
    addItem(0, -1, "root");
    addItem(0, "item1");
    addItem(0, "item2");
    addItem(0, "item3");
    
    auto items = m_db->getItems(0, 0, 0);
    EXPECT_EQ(3u, items.size());
}

TEST_F(LibraryDatabaseTest, GetItemsOffsetCount)
{
    addItem(0, -1, "root");
    addItem(0, "item1");
    addItem(0, "item2");
    addItem(0, "item3");
    
    // 1 item beginning at offset 1
    auto items = m_db->getItems(0, 1, 1);
    EXPECT_EQ(1u, items.size());
    
    // 2 item beginning at offset 1
    items = m_db->getItems(0, 1, 2);
    EXPECT_EQ(2u, items.size());
    
    // 3 item beginning at offset 1 -> results in 2 items
    items = m_db->getItems(0, 1, 3);
    EXPECT_EQ(2u, items.size());
    
    // all items beginning at offset 1 -> results in 2 items
    items = m_db->getItems(0, 1, 0);
    EXPECT_EQ(2u, items.size());
    
    // all items beginning at offset 3 -> results in 0 items
    items = m_db->getItems(0, 3, 0);
    EXPECT_EQ(0u, items.size());
}

}
}
