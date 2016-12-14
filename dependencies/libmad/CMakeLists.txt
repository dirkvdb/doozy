SET(MAJOR 0)
SET(MINOR 15)
SET(REVISION 1b)
SET(NAME libmad)
SET(VERSION ${MAJOR}.${MINOR}.${REVISION})
SET(PACKAGE ${NAME}-${VERSION})

IF (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    IF (HOST MATCHES i686-apple-darwin)
        SET(HOST i686)
        SET(FPM INTEL)
    ELSE ()
        SET(HOST x86_64)
        SET(FPM 64BIT)
    ENDIF ()
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Windows")
    SET(HOST x86_64)
    SET(FPM 64BIT)
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Linux")
    IF(HOST MATCHES "^arm-linux-.*")
        SET(FPM ARM)
    ELSE ()
        SET(HOST x86_64)
        SET(FPM 64BIT)
    ENDIF ()
ENDIF ()

EXTERNALPROJECT_ADD(${NAME}
    URL http://sourceforge.net/projects/mad/files/${NAME}/${VERSION}/${PACKAGE}.tar.gz TIMEOUT 30
    INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
    PATCH_COMMAND patch -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/frame_length.diff &&
                  ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/patch/CMakeLists.txt ${CMAKE_CURRENT_BINARY_DIR}/libmad-prefix/src/libmad/ &&
                  ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/patch/mad.pc.in ${CMAKE_CURRENT_BINARY_DIR}/libmad-prefix/src/libmad/
    CMAKE_ARGS -G ${CMAKE_GENERATOR}
               ${POSTFIX_CONFIG}
               -DFPM=${FPM}
               -DBUILD_SHARED_LIBS=OFF
               -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
               -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
               -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
)
