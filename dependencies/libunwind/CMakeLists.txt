SET(NAME libunwind)
SET(MAJOR 1)
SET(MINOR 1)
SET(PACKAGE ${NAME}-${MAJOR}.${MINOR})

EXTERNALPROJECT_ADD(${NAME}
    URL http://download.savannah.gnu.org/releases/${NAME}/${PACKAGE}.tar.gz TIMEOUT 30
    INSTALL_DIR ${CMAKE_BINARY_DIR}/local
    PATCH_COMMAND patch -p0 < ${CMAKE_CURRENT_SOURCE_DIR}/notests.patch && autoreconf --force --install
    CONFIGURE_COMMAND <SOURCE_DIR>/configure ${CROSS_ARGS} ${HOST_ARGS} --disable-dependency-tracking --prefix=<INSTALL_DIR> --disable-shared --disable-minidebuginfo
)
