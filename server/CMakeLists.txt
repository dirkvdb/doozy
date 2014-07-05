CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

IF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Gcc")
    SET(CMAKE_CXX_FLAGS_DEBUG "-rdynamic ")
    SET(CMAKE_CXX_FLAGS "-stdlib=libstdc++")
ENDIF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Gcc")

FIND_PACKAGE(PkgConfig)
PKG_CHECK_MODULES(SQLITE sqlite3 REQUIRED)

SET (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -Wall -Wfatal-errors")

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${IMAGE_INCLUDE_DIRS}
    ${AUDIO_INCLUDE_DIRS}
    ${UTILS_INCLUDE_DIRS}
    ${SQLITE_INCLUDE_DIRS}
    ${UPNPFRAMEWORK_INCLUDE_DIRS}
)

LINK_DIRECTORIES(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${IMAGE_LIBRARY_DIRS}
    ${AUDIO_LIBRARY_DIRS}
    ${UTILS_LIBRARY_DIRS}
    ${UPNPFRAMEWORK_LIBRARY_DIRS}
)

SET(LINK_LIBS
    ${IMAGE_LIBRARIES}
    ${UTILS_LIBRARIES}
    ${AUDIO_LIBRARIES}
    ${SQLITE_LIBRARIES}
    ${UPNPFRAMEWORK_LIBRARIES}
)

ADD_EXECUTABLE(doozyserver
    main.cpp
    crashhandler.h
    musiclibrary/album.h                    musiclibrary/album.cpp
    musiclibrary/albumart.h                 musiclibrary/albumart.cpp
    musiclibrary/filesystemmusiclibrary.h   musiclibrary/filesystemmusiclibrary.cpp
    musiclibrary/libraryitem.h              musiclibrary/libraryitem.cpp
    musiclibrary/musicdb.h                  musiclibrary/musicdb.cpp
    musiclibrary/musiclibraryinterface.h
    musiclibrary/musiclibraryfactory.h      musiclibrary/musiclibraryfactory.cpp
    musiclibrary/scanner.h                  musiclibrary/scanner.cpp
    musiclibrary/track.h                    musiclibrary/track.cpp
    ../common/settings.h                    ../common/settings.cpp
)

TARGET_LINK_LIBRARIES(doozyserver ${LINK_LIBS})
INSTALL(TARGETS doozyserver RUNTIME DESTINATION bin)

