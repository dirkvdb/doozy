SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
SET(TEST_DATA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/testdata)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/doozytestconfig.h.in ${CMAKE_BINARY_DIR}/doozytestconfig.h)

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${UTILS_INCLUDE_DIRS}
    ${AUDIO_INCLUDE_DIRS}
    ${RENDERER_INCLUDE_DIRS}
    ${UPNPFRAMEWORK_INCLUDE_DIRS}
    ${SERVER_INCLUDE_DIRS}
)

INCLUDE_DIRECTORIES(SYSTEM
    ${CMAKE_CURRENT_SOURCE_DIR}/../modules/sqlpp11/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../modules/sqlpp11-connector-sqlite3/include
)

LINK_DIRECTORIES(
    ${SERVER_LIBRARY_DIRS}
)

SET(LINK_LIBS
    audio
    image
    utils
    upnpframework
    ${SERVER_LIBRARIES}
    doozyrenderer
)

SET (TEST_INFRA
    gmock-gtest-all.cpp
    eventnotification.h
    eventlistenermock.h
    settingsmocks.h
    main.cpp
    ../common/settings.cpp
)

IF (UPNP_SERVER)
    SET (SERVER_TESTS
        librarytest.cpp
        librarydatabasetest.cpp
    )
ENDIF ()

ADD_EXECUTABLE(doozytest
    ${TEST_INFRA}
    ${SERVER_TESTS}
)

IF (HAVE_LIBCEC)
    ADD_EXECUTABLE(cectest
        cectest.cpp
    )

    TARGET_LINK_LIBRARIES(cectest
        doozyrenderer
    )
ENDIF ()

TARGET_LINK_LIBRARIES(doozytest
	${LINK_LIBS}
)

ADD_TEST(NAME DoozyTests COMMAND doozytest)
