SET(MAJOR 1)
SET(MINOR 2)
SET(REVISION 0)
SET(NAME libxdg-basedir)
SET(PACKAGE ${NAME}-${MAJOR}.${MINOR}.${REVISION})

IF (CMAKE_CROSSCOMPILING)
    SET (EXTRA_ARGS --host=${HOST})
ENDIF ()

EXTERNALPROJECT_ADD(${NAME}
    URL https://github.com/devnev/${NAME}/archive/${PACKAGE}.tar.gz TIMEOUT 30
    INSTALL_DIR ${CMAKE_BINARY_DIR}/local
    PATCH_COMMAND autoreconf --force --install
    CONFIGURE_COMMAND ${CROSS_ARGS} <SOURCE_DIR>/configure ${EXTRA_ARGS} --disable-dependency-tracking --prefix=<INSTALL_DIR> --disable-shared
)
