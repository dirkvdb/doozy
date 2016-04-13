#!/bin/bash

function checkresult {
    "$@"
    local status=$?
    if [ $status -ne 0 ]; then
        echo "error with $@ status=$status" >&2
        exit $status
    fi
    return $status
}

if [ "$#" -ne 1 ]; then
    echo "No toolchain provided. Choices: archarmv6|archarmv7|macv6|android|native|nativegcc6|mingw"
    exit 1
fi

# make sure the gas-preprocessor script is in the path on osx
#UNAME=`uname`
#if [ "${UNAME}" = "Darwin" ]; then
#    CURPATH=`pwd`
#    export PATH="${CURPATH}/cross:$PATH"
#fi

BUILD_GENERATOR="Unix Makefiles"

if [ "$1" = "native" ]; then
    export ARMARCH=native
    BUILD_GENERATOR="Ninja"
elif [ "$1" = "nativegcc6" ]; then
    export ARMARCH=gcc
elif [ "$1" = "mingw" ]; then
    export ARMARCH=mingw
    BUILD_GENERATOR="MSYS Makefiles"
elif [ "$1" = "archarmv6" ]; then
    export ARMARCH=armv6
    export PATH="/opt/x-tools6h/arm-unknown-linux-gnueabihf/bin:$PATH"
    export CROSS=arm-unknown-linux-gnueabihf-
    export CFLAGS="-march=armv6j -mfpu=vfp -mfloat-abi=hard -marm -O3"
    export HOST="arm-linux-gnueabi"
elif [ "$1" = "archarmv7" ]; then
    export ARMARCH=armv7
    export PATH="/opt/x-tools7h/arm-unknown-linux-gnueabihf/bin:$PATH"
    export CROSS=arm-unknown-linux-gnueabihf-
    export CFLAGS="-march=armv7-a -mfpu=vfpv3 -mfloat-abi=hard -O3"
    export HOST="arm-linux-gnueabi"
elif [ "$1" = "macv6" ]; then
    export ARMARCH=armv6
    export PATH="/usr/local/linaro/arm-linux-gnueabihf-raspbian/bin/:$PATH"
    export CROSS=arm-linux-gnueabihf-
    export CFLAGS="-march=armv6j -mfpu=vfp -mfloat-abi=hard -marm -O3"
    export HOST="arm-linux-gnueabi"
elif [ "$1" = "android" ]; then
    export ARMARCH=androidv7
    TOOLCHAIN="/Users/dirk/android-toolchain"
    export SYSROOT="$TOOLCHAIN/sysroot"
    export PATH="$TOOLCHAIN/bin/:$PATH"
    export CROSS=arm-linux-androideabi-
    export CFLAGS="-march=armv7-a -mfloat-abi=softfp -mfpu=vfp -O3"
    export CXXFLAGS="-fexceptions -frtti -DANDROID -DBACKWARD_SYSTEM_UNKNOWN"
    export HOST="arm-linux-androideabi"
    export LDFLAGS="$LDFLAGS -march=armv7-a -Wl,--fix-cortex-a8"
else
    echo "Unknown toolchain provided: $1. Choices: archarmv6|archarmv7|macv6|android"
    exit 1
fi

# cross compile dependencies
checkresult rm -rf ../build/dependencies
checkresult mkdir -p ../build/dependencies
cd ../build/dependencies
checkresult cmake -G "${BUILD_GENERATOR}" -DCMAKE_TOOLCHAIN_FILE=../../dependencies/toolchain-${ARMARCH}.make -DCMAKE_BUILD_TYPE=Release ../../dependencies
checkresult ninja
