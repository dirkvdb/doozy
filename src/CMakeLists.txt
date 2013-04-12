CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

IF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Gcc")
    SET(CMAKE_CXX_FLAGS_DEBUG "-rdynamic ")
ENDIF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Gcc")

SET (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -Wall -Wfatal-errors")

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${AUDIO_INCLUDE_DIRS}
    ${UTILS_INCLUDE_DIRS}
    ${UPNPFRAMEWORK_INCLUDE_DIRS}
)

LINK_DIRECTORIES(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${AUDIO_LIBRARY_DIRS}
    ${UTILS_LIBRARY_DIRS}
    ${UPNPFRAMEWORK_LIBRARY_DIRS}
)

SET(LINK_LIBS
    ${UTILS_LIBRARIES}
    ${AUDIO_LIBRARIES}
    ${UPNPFRAMEWORK_LIBRARIES}
)

ADD_EXECUTABLE(doozy
    main.cpp
    crashhandler.h
    devicedescriptions.h
    doozy.h                     doozy.cpp
    mediarendererdevice.h       mediarendererdevice.cpp
    playqueue.h                 playqueue.cpp
    settings.h                  settings.cpp
    typeconversions.h
)

TARGET_LINK_LIBRARIES(doozy ${LINK_LIBS})
INSTALL(TARGETS doozy RUNTIME DESTINATION bin)

