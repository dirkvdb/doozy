SET(MAJOR 1)
SET(MINOR 6)
SET(REVISION 26)
SET(NAME libpng)
SET(PACKAGE ${NAME}-${MAJOR}.${MINOR}.${REVISION})

EXTERNALPROJECT_ADD(${NAME}
    DEPENDS zlib
    URL http://sourceforge.net/projects/${NAME}/files/libpng16/${MAJOR}.${MINOR}.${REVISION}/${PACKAGE}.tar.gz TIMEOUT 30
    INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
    CONFIGURE_COMMAND WANT_AUTOMAKE=latest <SOURCE_DIR>/configure ${BUILD} ${CROSS_ARGS} --disable-dependency-tracking --host=${HOST} --prefix=<INSTALL_DIR> --disable-shared
)
