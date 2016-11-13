SET(MAJOR 1)
SET(MINOR 10)
SET(REVISION 6)
SET(NAME asio)
SET(VERSION ${MAJOR}.${MINOR}.${REVISION})
SET(PACKAGE ${NAME}-${MAJOR}.${MINOR}.${REVISION})

EXTERNALPROJECT_ADD(${NAME}
    URL http://downloads.sourceforge.net/project/asio/asio/${VERSION}%20%28Stable%29/${PACKAGE}.tar.bz2 TIMEOUT 180
    INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
    PATCH_COMMAND patch -p1 < ${CMAKE_CURRENT_SOURCE_DIR}/gcc.patch && autoreconf --force --install
    CONFIGURE_COMMAND <SOURCE_DIR>/configure ${BUILD} ${CROSS_ARGS} --host=${HOST} --prefix=<INSTALL_DIR>
        --without-boost
        --without-openssl
        --disable-dependency-tracking
)

