set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set(TEST_DATA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/testdata)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doozytestconfig.h.in ${CMAKE_BINARY_DIR}/doozytestconfig.h)

include_directories(SYSTEM
    ${CMAKE_CURRENT_SOURCE_DIR}/testframework
)

if (DOOZY_RENDERER)
    list(APPEND LINK_LIBS doozyrenderer)
    set(RENDERER_TESTS renderer.playqueuetest.cpp renderertest.cpp)
endif ()

if (DOOZY_SERVER)
    list(APPEND LINK_LIBS doozyserver)
    set(SERVER_TESTS librarytest.cpp librarydatabasetest.cpp)
endif ()

set(TEST_INFRA
    testframework/gmock-gtest-all.cpp
    ../doozydevicefactory
    eventnotification.h
    eventlistenermock.h
    settingsmocks.h
    main.cpp
)

add_executable(doozytest
    ${TEST_INFRA}
    ${SERVER_TESTS}
    ${RENDERER_TESTS}
)

if (HAVE_LIBCEC)
    add_executable(cectest
        cectest.cpp
    )

    target_include_directories(cectest SYSTEM PRIVATE
        ${LIBCEC_INCLUDE_DIR}
    )

    target_link_libraries(cectest
        doozyrenderer
        utils
    )
endif ()

target_link_libraries(doozytest
    ${LINK_LIBS}
    doozycommon
)

add_test(NAME DoozyTests COMMAND doozytest)
