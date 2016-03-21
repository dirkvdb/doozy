SET(MAJOR 1)
SET(MINOR 3)
SET(REVISION 1)
SET(NAME flac)
SET(PACKAGE ${NAME}-${MAJOR}.${MINOR}.${REVISION})

IF (HOST MATCHES i686-apple-darwin)
    SET(CFG_OPT --disable-asm-optimizations)
ENDIF ()

EXTERNALPROJECT_ADD(${NAME}
    URL http://downloads.xiph.org/releases/flac/${PACKAGE}.tar.xz TIMEOUT 30
    DOWNLOAD_NAME ${PACKAGE}.tar.gz # hack to support cmake 2.8 which does not recognize xz extension
    INSTALL_DIR ${CMAKE_BINARY_DIR}/local
    CONFIGURE_COMMAND <SOURCE_DIR>/configure ${CROSS_ARGS} ${CFG_OPT} --disable-dependency-tracking --host=${HOST} --prefix=<INSTALL_DIR> --disable-shared --disable-ogg --disable-xmms-plugin --disable-doxygen-docs
)
