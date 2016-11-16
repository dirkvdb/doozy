SET(MAJOR 1)
SET(MINOR 5)
SET(REVISION 1)
SET(NAME libjpeg-turbo)
SET(PACKAGE ${NAME}-${MAJOR}.${MINOR}.${REVISION})

IF (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    SET (NASMOPT "NASM=/usr/local/bin/nasm")
ENDIF ()

IF (WIN32)
    EXTERNALPROJECT_ADD(${NAME}
        URL http://sourceforge.net/projects/${NAME}/files/${VERSION}/${PACKAGE}.tar.gz/download TIMEOUT 30
        INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
        CMAKE_ARGS -G ${CMAKE_GENERATOR} ${EXTRA_OPTS} -DENABLE_SHARED=FALSE -DWITH_TURBOJPEG=FALSE -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    )
ELSE ()
    EXTERNALPROJECT_ADD(${NAME}
        URL http://sourceforge.net/projects/${NAME}/files/${VERSION}/${PACKAGE}.tar.gz/download TIMEOUT 30
        INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
        PATCH_COMMAND WANT_AUTOMAKE=latest autoreconf --force --install
        CONFIGURE_COMMAND WANT_AUTOMAKE=latest <SOURCE_DIR>/configure ${BUILD} ${CROSS_ARGS} ${NASMOPT} --disable-dependency-tracking --host=${HOST} --prefix=<INSTALL_DIR> --disable-shared
    )
ENDIF ()