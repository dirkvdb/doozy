if (NOT WITHOUT_CEC)
    pkg_check_modules(LIBCEC_PKG libcec)
    if (LIBCEC_PKG_FOUND)
        option(HAVE_LIBCEC "CEC support" ON)

        find_path(LIBCEC_INCLUDE_DIR NAMES libcec/cec.h HINTS ${LIBCEC_PKG_INCLUDE_DIRS})

        find_library(LIBCEC_LIBRARY
            NAMES ${LIBCEC_PKG_LIBRARIES}
            HINTS ${LIBCEC_PKG_LIBRARY_DIRS}
        )

        if (NOT LIBCEC_INCLUDE_DIR)
            message(FATAL_ERROR "Failed to find cec headers in ${LIBCEC_PKG_INCLUDE_DIRS}")
        endif ()

        set(LIBCEC_INCLUDE_DIR ${LIBCEC_INCLUDE_DIR}/libcec PARENT_SCOPE)
        set(LIBCEC_INCLUDE_DIR ${LIBCEC_INCLUDE_DIR}/libcec)
        set(LIBCEC_LIBRARIES ${LIBCEC_LIBRARY} ${CMAKE_DL_LIBS})
    endif ()
endif ()

add_library(doozyrenderer
    devicedescriptions.h
    renderersettings.h          renderersettings.cpp
    mediarendererdevice.h       mediarendererdevice.cpp
    playqueue.h                 playqueue.cpp
    typeconversions.h
)

target_include_directories(doozyrenderer PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(doozyrenderer
    audio
    image
    upnpframework
    doozycommon
)

if (HAVE_LIBCEC)
    target_sources(doozyrenderer PRIVATE ceccontrol.h ceccontrol.cpp)

    target_include_directories(doozyrenderer SYSTEM PRIVATE
        ${LIBCEC_INCLUDE_DIR}
    )

    target_link_libraries(doozyrenderer
        ${LIBCEC_LIBRARIES}
    )
endif ()
