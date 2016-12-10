CMAKE_MINIMUM_REQUIRED(VERSION 3.1)
CMAKE_POLICY(VERSION 3.1)
INCLUDE(ExternalProject)

message (STATUS "Host: ${CMAKE_HOST_SYSTEM}")
message (STATUS "Target: ${CMAKE_SYSTEM_NAME}")
message (STATUS "Cross compiling: ${CMAKE_CROSSCOMPILING}")

IF (CMAKE_CROSSCOMPILING)
    IF (HOST MATCHES i686-apple-darwin)
        SET(LDFLAGS "-m32")
    ENDIF ()

    SET(CROSS_ARGS CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER} AR=${CMAKE_AR} RANLIB=${CMAKE_RANLIB} LDFLAGS=${LDFLAGS})
    SET(HOST_ARGS --host=${HOST})
ENDIF ()

IF (NOT HOST MATCHES "^.*musl.*" AND NOT MINGW)
    # musl toolchain is used for static builds. libcec only builds shared library
    IF (NOT CMAKE_COMPILER_IS_GNUCC AND NOT APPLE)
        # gcc fails to compile libcec
        ADD_SUBDIRECTORY(libplatform)
        ADD_SUBDIRECTORY(libcec)
    ENDIF ()
    ADD_SUBDIRECTORY(libxdg-basedir)
ENDIF ()

IF (UNIX AND NOT APPLE)
    ADD_SUBDIRECTORY(libunwind)
ENDIF ()

ADD_SUBDIRECTORY(zlib)
ADD_SUBDIRECTORY(ffmpeg)
ADD_SUBDIRECTORY(boost)
ADD_SUBDIRECTORY(beast)
ADD_SUBDIRECTORY(libjpeg-turbo)
ADD_SUBDIRECTORY(libmad)

IF (CMAKE_COMPILER_IS_GNUCC AND APPLE)
    # homebrew gcc does not like coreaudio includes
    MESSAGE(STATUS "No audio support when using gcc on apple")
ELSE ()
    ADD_SUBDIRECTORY(openal)
ENDIF ()

ADD_SUBDIRECTORY(flac)
ADD_SUBDIRECTORY(sqlite)
ADD_SUBDIRECTORY(libpng)
ADD_SUBDIRECTORY(taglib)
ADD_SUBDIRECTORY(benchmark)

IF (NOT CMAKE_SYSTEM_NAME MATCHES "Darwin" AND NOT CMAKE_SYSTEM_NAME MATCHES "Windows")
    ADD_SUBDIRECTORY(alsa-lib)
    ADD_DEPENDENCIES(openal-soft alsa-lib)
ENDIF ()

