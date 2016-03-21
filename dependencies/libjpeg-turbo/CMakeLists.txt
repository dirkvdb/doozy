SET(MAJOR 1)
SET(MINOR 4)
SET(REVISION 2)
SET(NAME libjpeg-turbo)
SET(PACKAGE ${NAME}-${MAJOR}.${MINOR}.${REVISION})

IF (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    SET (NASMOPT "NASM=/usr/local/bin/nasm")
ENDIF ()

EXTERNALPROJECT_ADD(${NAME}
    URL http://sourceforge.net/projects/${NAME}/files/${VERSION}/${PACKAGE}.tar.gz/download TIMEOUT 30
    INSTALL_DIR ${CMAKE_BINARY_DIR}/local
    PATCH_COMMAND WANT_AUTOMAKE=latest autoreconf --force --install
    CONFIGURE_COMMAND WANT_AUTOMAKE=latest <SOURCE_DIR>/configure ${BUILD} ${CROSS_ARGS} ${NASMOPT} --disable-dependency-tracking --host=${HOST} --prefix=<INSTALL_DIR> --disable-shared
)
