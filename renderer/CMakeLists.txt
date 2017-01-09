pkg_check_modules(LIBCEC_PKG libcec)
if (LIBCEC_PKG_FOUND)
    option(HAVE_LIBCEC "CEC support" ON)

    find_path(LIBCEC_INCLUDE_DIR
        NAMES cec.h
        PATHS ${LIBCEC_PKG_INCLUDE_DIRS}
    )

    find_library(LIBCEC_LIBRARY
        NAMES ${LIBCEC_PKG_LIBRARIES}
        PATHS ${LIBCEC_PKG_LIBRARY_DIRS}
    )

    if (NOT LIBCEC_INCLUDE_DIR)
        message(FATAL_ERROR "Failed to find cec headers in ${LIBCEC_PKG_INCLUDE_DIRS}")
    endif ()

    set(LIBCEC_LIBRARIES ${LIBCEC_LIBRARY} ${CMAKE_DL_LIBS})
endif ()

set (RENDERER_SRC
    devicedescriptions.h
    renderersettings.h          renderersettings.cpp
    mediarendererdevice.h       mediarendererdevice.cpp
    playqueue.h                 playqueue.cpp
    typeconversions.h
)

if (HAVE_LIBCEC)
    list(APPEND RENDERER_SRC ceccontrol.h ceccontrol.cpp)
endif ()

add_library(doozyrenderer
    ${RENDERER_SRC}
)

target_include_directories(doozyrenderer PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

target_include_directories(doozyrenderer SYSTEM PRIVATE
    ${LIBCEC_INCLUDE_DIR}
)

target_link_libraries(doozyrenderer
    audio
    image
    upnpframework
    doozycommon
    ${LIBCEC_LIBRARIES}
)
