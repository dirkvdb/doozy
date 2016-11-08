SET(MAJOR 3)
SET(MINOR 2)
SET(REVISION 0)
SET(NAME ffmpeg)
#SET(PACKAGE ${NAME}-${MAJOR}.${MINOR}.${REVISION})
SET(PACKAGE ${NAME}-${MAJOR}.${MINOR})

IF (CMAKE_BUILD_TYPE MATCHES Debug)
    SET (DBGSETTING --enable-debug=3)
ENDIF ()

IF (CMAKE_CROSSCOMPILING)
    IF (CMAKE_SYSTEM_NAME MATCHES "Darwin")
        IF (HOST MATCHES i686-apple-darwin)
            SET(ARCHSETTING --arch=i686)
            SET(COMPSETTING --cc=${CMAKE_C_COMPILER}\ -m32 --disable-iconv)
        ELSE ()
            SET(ARCHSETTING --arch=x86_64)
            SET(COMPSETTING --cc=${CMAKE_C_COMPILER} --disable-iconv)
        ENDIF ()

        # make sure gas-preprocessor.pl is in the path
        SET($ENV{PATH} "$ENV{PATH}:${CMAKE_CURRENT_SOURCE_DIR}")
    ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Windows")
        SET(ARCHSETTING --arch=x86_64)
        SET(COMPSETTING --enable-cross-compile --cross-prefix=${CROSS})
        SET(TARGETSETTING --target-os=mingw32)
    ELSE ()
        SET(TARGETSETTING --target-os=linux)
        IF(HOST MATCHES "^arm-linux-.*")
            SET(ARCHSETTING --arch=armel)
            SET(COMPSETTING --enable-cross-compile --enable-armv6 --cc=${CMAKE_C_COMPILER} --ar=${CMAKE_AR} --ranlib=${CMAKE_RANLIB})
        ELSE ()
            SET(ARCHSETTING --arch=x86_64)
            SET(COMPSETTING --cc=${CMAKE_C_COMPILER})

            IF (HOST MATCHES "^x86_64-linux-musl*")
                SET(COMPSETTING --enable-cross-compile ${COMPSETTING})
            ENDIF ()
        ENDIF ()
    ENDIF ()
ENDIF ()

EXTERNALPROJECT_ADD(${NAME}
    URL http://www.ffmpeg.org/releases/${PACKAGE}.tar.bz2 TIMEOUT 240
    INSTALL_DIR ${CMAKE_BINARY_DIR}/local
    CONFIGURE_COMMAND <SOURCE_DIR>/configure ${TARGETSETTING} ${COMPSETTING} ${ARCHSETTING} ${DBGSETTING} --prefix=<INSTALL_DIR> --disable-shared --disable-avdevice --disable-doc --disable-htmlpages --disable-manpages --disable-programs
        --disable-encoders --disable-muxers --disable-decoders --disable-swscale --disable-iconv
        --enable-gpl --enable-network
        --disable-everything
        --enable-protocol=http
        --enable-protocol=file
        --enable-decoder=aac
        --enable-decoder=alac
        --enable-decoder=ac3
        --enable-decoder=flac
        --enable-decoder=mp3
        --enable-decoder=wmav1
        --enable-decoder=wmav2
        --enable-decoder=wmalossless
        --enable-decoder=wmapro
        --enable-decoder=pcm_u8
        --enable-decoder=pcm_s8
        --enable-decoder=vorbis
        --enable-parser=ac3
        --enable-parser=aac
        --enable-parser=aac_latm
        --enable-parser=flac
        --enable-parser=mpegaudio
        --enable-parser=vorbis
        --enable-demuxer=aac
        --enable-demuxer=ac3
        --enable-demuxer=eac3
        --enable-demuxer=flac
        --enable-demuxer=mp3
        --enable-demuxer=mpc
        --enable-demuxer=wav
        --enable-demuxer=pcm_u8
        --enable-demuxer=pcm_s8
        --enable-demuxer=xwma
        --enable-demuxer=mov
        --enable-bsf=aac_adtstoasc
)
