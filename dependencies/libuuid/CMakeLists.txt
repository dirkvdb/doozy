SET(MAJOR 1)
SET(MINOR 0)
SET(REVISION 3)
SET(NAME libuuid)
SET(VERSION ${MAJOR}.${MINOR}.${REVISION})
SET(PACKAGE ${NAME}-${VERSION})

IF (CMAKE_CROSSCOMPILING)
    SET (EXTRA_ARGS --host=${HOST})
ENDIF ()

EXTERNALPROJECT_ADD(${NAME}
    URL http://sourceforge.net/projects/${NAME}/files/${PACKAGE}.tar.gz/download TIMEOUT 6000
    INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
    CONFIGURE_COMMAND ${CROSS_ARGS} <SOURCE_DIR>/configure
        ${EXTRA_ARGS}
        --disable-dependency-tracking
        --prefix=<INSTALL_DIR>
        --disable-shared
)
