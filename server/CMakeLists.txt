CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

FIND_PACKAGE(PkgConfig)
PKG_CHECK_MODULES(SQLITE sqlite3 REQUIRED)

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../modules/sqlpp11/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../modules/sqlpp11-connector-sqlite3/include
    ${CMAKE_CURRENT_BINARY_DIR}
    ${IMAGE_INCLUDE_DIRS}
    ${AUDIO_INCLUDE_DIRS}
    ${UTILS_INCLUDE_DIRS}
    ${SQLITE_INCLUDE_DIRS}
    ${UPNPFRAMEWORK_INCLUDE_DIRS}
)

LINK_DIRECTORIES(
    ${SQLITE_LINK_DIRECTORIES}
)

SET (SERVER_LIBRARIES doozyserver sqlpp11-connector-sqlite3 ${SQLITE_LIBRARIES})

ADD_LIBRARY(doozyserver
    filerequesthandler.h				filerequesthandler.cpp
    mediaserverdevice.h                 mediaserverdevice.cpp
    mimetypes.h                         mimetypes.cpp
    server.h                            server.cpp
    serversettings.h                    serversettings.cpp
    library/database.h
    library/md5.h                       library/md5.cpp
    library/filesystemmusiclibrary.h    library/filesystemmusiclibrary.cpp
    library/libraryitem.h
    library/musicdb.h                   library/musicdb.cpp
    library/musiclibraryinterface.h
    library/scanner.h                   library/scanner.cpp
)

SOURCE_GROUP(library REGULAR_EXPRESSION library/)

