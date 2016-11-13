SET(MAJOR 1)
SET(MINOR 9)
SET(REVISION 1)
SET(NAME libuv)
SET(PACKAGE ${NAME}-${MAJOR}.${MINOR}.${REVISION})

EXTERNALPROJECT_ADD(${NAME}
    URL http://github.com/${NAME}/${NAME}/archive/v${MAJOR}.${MINOR}.${REVISION}.tar.gz
    INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
    PATCH_COMMAND <SOURCE_DIR>/autogen.sh
    CONFIGURE_COMMAND <SOURCE_DIR>/configure ${BUILD} ${CROSS_ARGS} --disable-dependency-tracking --prefix=<INSTALL_DIR> --disable-shared
)
