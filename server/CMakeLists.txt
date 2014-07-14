CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

IF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Gcc")
    SET(CMAKE_CXX_FLAGS_DEBUG "-rdynamic ")
    SET(CMAKE_CXX_FLAGS "-stdlib=libstdc++")
ENDIF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Gcc")

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

ADD_LIBRARY(doozyserver
    server.h                           server.cpp
    library/filesystemmusiclibrary.h   library/filesystemmusiclibrary.cpp
    library/libraryitem.h              library/libraryitem.cpp
    library/musicdb.h                  library/musicdb.cpp
    library/musiclibraryinterface.h
    library/musiclibraryfactory.h      library/musiclibraryfactory.cpp
    library/scanner.h                  library/scanner.cpp
    ../common/settings.h               ../common/settings.cpp
)

SOURCE_GROUP(library REGULAR_EXPRESSION library/)

