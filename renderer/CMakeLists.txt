PKG_CHECK_MODULES(LIBCEC_PKG libcec)
IF (LIBCEC_PKG_FOUND)
    OPTION(HAVE_LIBCEC "CEC support" ON)

    FIND_PATH(LIBCEC_INCLUDE_DIR
        NAMES cec.h
        PATHS ${LIBCEC_PKG_INCLUDE_DIRS}
    )

    FIND_LIBRARY(LIBCEC_LIBRARY
        NAMES ${LIBCEC_PKG_LIBRARIES}
        PATHS ${LIBCEC_PKG_LIBRARY_DIRS}
    )

    SET(LIBCEC_LIBRARIES ${LIBCEC_LIBRARY} -ldl)
ENDIF ()

INCLUDE_DIRECTORIES(SYSTEM
    ${LIBCEC_INCLUDE_DIR}
)

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${AUDIO_INCLUDE_DIRS}
    ${IMAGE_INCLUDE_DIRS}
    ${UTILS_INCLUDE_DIRS}
    ${UPNPFRAMEWORK_INCLUDE_DIRS}
)

INCLUDE_DIRECTORIES(SYSTEM
    ${UPNPFRAMEWORK_SYS_INCLUDE_DIRS}
)

SET (RENDERER_INCLUDE_DIRS ${LIBCEC_INCLUDE_DIR})
SET (RENDERER_LIBRARIES doozyrenderer)

SET (RENDERER_SRC
    devicedescriptions.h
    renderersettings.h          renderersettings.cpp
    mediarendererdevice.h       mediarendererdevice.cpp
    playqueue.h                 playqueue.cpp
    typeconversions.h
)

IF (HAVE_LIBCEC)
    LIST(APPEND RENDERER_SRC
        ceccontrol.h ceccontrol.cpp
    )
ENDIF ()

ADD_LIBRARY(doozyrenderer
    ${RENDERER_SRC}
)

TARGET_LINK_LIBRARIES(doozyrenderer
    ${LIBCEC_LIBRARIES}
)