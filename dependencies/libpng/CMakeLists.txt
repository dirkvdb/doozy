SET(MAJOR 1)
SET(MINOR 6)
SET(REVISION 23)
SET(NAME libpng)
SET(PACKAGE ${NAME}-${MAJOR}.${MINOR}.${REVISION})

SET (SYSROOT <INSTALL_DIR>)

IF (MINGW)
    # it fails on sysroots starting with c:/ (not considered an absolute path)
    SET (SYSROOT "/c/msys64/home/dvdboer/projects/doozy/deps/local")
    STRING(REPLACE "c:/" "/c/" SYSROOT ${SYSROOT})
ENDIF()

EXTERNALPROJECT_ADD(${NAME}
    DEPENDS zlib
    URL http://sourceforge.net/projects/${NAME}/files/libpng16/${MAJOR}.${MINOR}.${REVISION}/${PACKAGE}.tar.gz TIMEOUT 30
    INSTALL_DIR ${CMAKE_BINARY_DIR}/local
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DPNG_SHARED=OFF -DPNG_TESTS=OFF -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
)
