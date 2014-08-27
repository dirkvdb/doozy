CREATE TABLE objects(
    Id INTEGER PRIMARY KEY,
    ParentId INTEGER NOT NULL,
    RefId INTEGER,
    Name TEXT NOT NULL,
    Class TEXT,
    MetaData INTEGER,
    FOREIGN KEY (MetaData) REFERENCES metadata(id)
);

CREATE TABLE metadata(
    Id INTEGER PRIMARY KEY,
    Album TEXT,
    Artist TEXT,
    Title TEXT,
    AlbumArtist TEXT,
    Genre TEXT,
    Composer TEXT,
    Year INTEGER,
    TrackNr INTEGER,
    DiscNr INTEGER,
    AlbumOrder INTEGER,
    Duration INTEGER,
    MimeType TEXT,
    BitRate INTEGER,
    SampleRate INTEGER,
    Channels INTEGER,
    FileSize INTEGER,
    DateAdded INTEGER,
    ModifiedTime INTEGER,
    Thumbnail TEXT,
    FilePath TEXT
);
