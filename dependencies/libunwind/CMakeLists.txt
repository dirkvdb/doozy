SET(NAME libunwind)
SET(MAJOR 1)
SET(MINOR 2)
SET(REVISION 1)
SET(PACKAGE ${NAME}-${MAJOR}.${MINOR}.${REVISION})

EXTERNALPROJECT_ADD(${NAME}
    URL http://download.savannah.gnu.org/releases/${NAME}/${PACKAGE}.tar.gz TIMEOUT 30
    INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
    PATCH_COMMAND patch -p0 < ${CMAKE_CURRENT_SOURCE_DIR}/notests.patch && autoreconf --force --install
    CONFIGURE_COMMAND <SOURCE_DIR>/configure ${CROSS_ARGS} ${HOST_ARGS} --disable-dependency-tracking --prefix=<INSTALL_DIR> --disable-shared --disable-minidebuginfo
)
