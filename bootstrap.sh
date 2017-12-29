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
    echo "No toolchain provided. Choices: archarmv6|armv7|android|native|nativegcc6|mingw|clang"
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
    export ARCH=native
    BUILD_GENERATOR="Unix Makefiles"
elif [ "$1" = "nativegcc6" ]; then
    export ARCH=gcc
    BUILD_GENERATOR="Unix Makefiles"
elif [ "$1" = "clang" ]; then
    export ARCH=clang
    BUILD_GENERATOR="Ninja"
elif [ "$1" = "mingw" ]; then
    export ARCH=mingw
    BUILD_GENERATOR="Unix Makefiles"
elif [ "$1" = "archarmv6" ]; then
    export ARCH=armv6
    export PATH="/opt/x-tools6h/arm-unknown-linux-gnueabihf/bin:$PATH"
    export CROSS=arm-unknown-linux-gnueabihf-
    export CFLAGS="-march=armv6j -mfpu=vfp -mfloat-abi=hard -marm -O3"
    export HOST="arm-linux-gnueabi"
elif [ "$1" = "armv7" ]; then
    export ARCH=armv7
    export PATH="/opt/x-tools7h/arm-unknown-linux-gnueabihf/bin:/opt/armv7-rpi2-linux-gnueabihf/bin/:$PATH"
    export HOST="arm-linux-gnueabihf"
elif [ "$1" = "android" ]; then
    export ARCH=androidv7
    TOOLCHAIN="/Users/dirk/android-toolchain"
    export SYSROOT="$TOOLCHAIN/sysroot"
    export PATH="$TOOLCHAIN/bin/:$PATH"
    export CROSS=arm-linux-androideabi-
    export CFLAGS="-march=armv7-a -mfloat-abi=softfp -mfpu=vfp -O3"
    export CXXFLAGS="-fexceptions -frtti -DANDROID -DBACKWARD_SYSTEM_UNKNOWN"
    export HOST="arm-linux-androideabi"
    export LDFLAGS="$LDFLAGS -march=armv7-a -Wl,--fix-cortex-a8"
else
    echo "Unknown toolchain provided: $1. Choices: archarmv6|armv7|android|native|nativegcc6|mingw|clang"
    exit 1
fi

# cross compile dependencies
checkresult mkdir -p ./build/deps-${ARCH}
cd ./build/deps-${ARCH}
PWD=`pwd`
checkresult cmake -G "${BUILD_GENERATOR}" \
                  -DHOST=${HOST} \
                  -DCMAKE_PREFIX_PATH=${PWD}/../local-toolchain-${ARCH} \
                  -DCMAKE_INSTALL_PREFIX=${PWD}/../local-toolchain-${ARCH} \
                  -DCMAKE_TOOLCHAIN_FILE=${PWD}/../../dependencies/toolchain-${ARCH}.make \
                  -DCMAKE_BUILD_TYPE=Release \
                  ../../dependencies
checkresult cmake --build .
