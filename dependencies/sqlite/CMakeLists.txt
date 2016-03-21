SET(NAME sqlite)
SET(VERSION 3081101)
SET(PACKAGE ${NAME}-autoconf-${VERSION})

EXTERNALPROJECT_ADD(${NAME}
    URL http://www.sqlite.org/2015/${PACKAGE}.tar.gz TIMEOUT 30
    INSTALL_DIR ${CMAKE_BINARY_DIR}/local
    PATCH_COMMAND autoreconf --force --install
    CONFIGURE_COMMAND <SOURCE_DIR>/configure ${CROSS_ARGS} --disable-dependency-tracking --host=${HOST} --prefix=<INSTALL_DIR> --disable-shared
)
