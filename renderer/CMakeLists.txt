CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${AUDIO_INCLUDE_DIRS}
    ${IMAGE_INCLUDE_DIRS}
    ${UTILS_INCLUDE_DIRS}
    ${UPNPFRAMEWORK_INCLUDE_DIRS}
)

SET (RENDERER_LIBRARIES doozyrenderer)

ADD_LIBRARY(doozyrenderer
    renderer.h                  renderer.cpp
    renderersettings.h          renderersettings.cpp
    mediarendererdevice.h       mediarendererdevice.cpp
    playqueue.h                 playqueue.cpp
    typeconversions.h
)
