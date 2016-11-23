find_package(PkgConfig)
pkg_check_modules(SQLITE sqlite3 REQUIRED)

add_library(doozyserver
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

target_include_directories(doozyserver PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
    $<INSTALL_INTERFACE:include>
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
)

target_include_directories(doozyserver SYSTEM PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../modules/sqlpp11/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../modules/sqlpp11-connector-sqlite3/include
    ${SQLITE_INCLUDE_DIRS}
    ${UPNPFRAMEWORK_INCLUDE_DIRS}
)

source_group(library REGULAR_EXPRESSION library/)

