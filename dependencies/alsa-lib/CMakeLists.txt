SET(MAJOR 1)
SET(MINOR 1)
SET(REVISION 3)
SET(NAME alsa-lib)
SET(PACKAGE ${NAME}-${MAJOR}.${MINOR}.${REVISION})

EXTERNALPROJECT_ADD(${NAME}
    URL ftp://ftp.alsa-project.org/pub/lib/${PACKAGE}.tar.bz2 TIMEOUT 30
    INSTALL_DIR ${CMAKE_PREFIX_PATH}
    CONFIGURE_COMMAND <SOURCE_DIR>/configure
            ${CROSS_ARGS}
            --disable-dependency-tracking
            --host=${HOST}
            --prefix=/usr
            --disable-shared
            --disable-python
            --disable-rawmidi
            --disable-ucm
            --disable-alisp
            --with-libdl=no
            --disable-old-symbols
    INSTALL_COMMAND ${CMAKE_MAKE_PROGRAM} DESTDIR=<INSTALL_DIR> install
)
