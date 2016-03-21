SET(MAJOR 1)
SET(MINOR 6)
SET(REVISION 19)
SET(NAME libupnp)
SET(PACKAGE ${NAME}-${MAJOR}.${MINOR}.${REVISION})

EXTERNALPROJECT_ADD(${NAME}
    URL http://sourceforge.net/projects/pupnp/files/pupnp/libUPnP%201.6.19/${PACKAGE}.tar.bz2 TIMEOUT 30
    INSTALL_DIR ${CMAKE_BINARY_DIR}/local
    PATCH_COMMAND patch -p1 < ${CMAKE_CURRENT_SOURCE_DIR}/mingw.patch && patch -p1 < ${CMAKE_CURRENT_SOURCE_DIR}/tp.patch && autoreconf --force --install
    CONFIGURE_COMMAND <SOURCE_DIR>/configure ${CROSS_ARGS} --disable-dependency-tracking --host=${HOST} --prefix=<INSTALL_DIR> --disable-shared --enable-ipv6 --disable-samples --disable-blocking-tcp-connections
    BUILD_IN_SOURCE 1
)
