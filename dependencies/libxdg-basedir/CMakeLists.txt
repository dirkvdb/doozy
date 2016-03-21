SET(MAJOR 1)
SET(MINOR 2)
SET(REVISION 0)
SET(NAME libxdg-basedir)
SET(PACKAGE ${NAME}-${MAJOR}.${MINOR}.${REVISION})

EXTERNALPROJECT_ADD(${NAME}
    URL https://github.com/devnev/${NAME}/archive/${PACKAGE}.tar.gz TIMEOUT 30
    INSTALL_DIR ${CMAKE_BINARY_DIR}/local
    PATCH_COMMAND autoreconf --force --install
    CONFIGURE_COMMAND <SOURCE_DIR>/configure ${CROSS_ARGS} --disable-dependency-tracking --host=${HOST} --prefix=<INSTALL_DIR> --disable-shared
)
