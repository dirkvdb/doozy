SET(MAJOR 7)
SET(MINOR 48)
SET(REVISION 0)
SET(NAME curl)
SET(PACKAGE ${NAME}-${MAJOR}.${MINOR}.${REVISION})

EXTERNALPROJECT_ADD(${NAME}
    URL https://curl.haxx.se/download/${PACKAGE}.tar.gz TIMEOUT 30
    DEPENDS zlib
    INSTALL_DIR ${CMAKE_BINARY_DIR}/local
    CONFIGURE_COMMAND <SOURCE_DIR>/configure ${BUILD} ${CROSS_ARGS} --disable-dependency-tracking --prefix=<INSTALL_DIR> --disable-shared
    --without-nghttp2
    --without-libssh2
    --without-libpsl
    --without-cyassl
    --without-polarssl
    --without-gnutls
    --without-darwinssl
    --without-winssl
    --disable-manual
    --disable-gopher
    --disable-smtp
    --disable-smb
    --disable-imap
    --disable-pop3
    --disable-tftp
    --disable-telnet
    --disable-dict
    --disable-ldap
    --disable-file
    --disable-ftp
    --without-ca-path
)
