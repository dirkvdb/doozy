SET(MAJOR 1)
SET(MINOR 10)
SET(REVISION 6)
SET(NAME asio)
SET(VERSION ${MAJOR}.${MINOR}.${REVISION})
SET(PACKAGE ${NAME}-${MAJOR}.${MINOR}.${REVISION})

EXTERNALPROJECT_ADD(${NAME}
    URL http://downloads.sourceforge.net/project/asio/asio/${VERSION}%20%28Stable%29/${PACKAGE}.tar.bz2 TIMEOUT 180
    INSTALL_DIR ${CMAKE_BINARY_DIR}/local
    CONFIGURE_COMMAND WANT_AUTOMAKE=latest <SOURCE_DIR>/configure ${BUILD} ${CROSS_ARGS} --host=${HOST} --prefix=<INSTALL_DIR>
        --disable-shared
        --without-boost
        --without-openssl
        --disable-dependency-tracking
)
