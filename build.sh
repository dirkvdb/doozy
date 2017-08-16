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
    echo "No toolchain provided. Choices: archarmv6|archarmv7|macv6|android|native|nativegcc6|mingw|clang4"
    exit 1
fi

platform=`uname`
platformopts=""

if [[ "$platform" == 'Linux' ]]; then
    echo "Linux"
    platformopts="-DWITHOUT_CEC=OFF"
elif [[ "$platform" == 'Darwin' && "$1" = "native" ]]; then
    echo "Apple"
    platformopts="-DWITHOUT_CEC=ON"
else
    platformopts="-DWITHOUT_CEC=OFF"
fi

if [ "$1" = "native" ]; then
    export ARCH=native
elif [ "$1" = "nativegcc6" ]; then
    export ARCH=gcc
elif [ "$1" = "clang4" ]; then
    export ARCH=clang4
elif [ "$1" = "mingw" ]; then
    export ARCH=mingw
elif [ "$1" = "archarmv6" ]; then
    export ARCH=armv6
    export PATH="/opt/x-tools6h/arm-unknown-linux-gnueabihf/bin:$PATH"
    export CROSS=arm-unknown-linux-gnueabihf-
    export CFLAGS="-march=armv6j -mfpu=vfp -mfloat-abi=hard -marm -O3"
    export HOST="arm-linux-gnueabi"
elif [ "$1" = "armv7" ]; then
    export ARCH=armv7
    export PATH="/opt/x-tools7h/arm-unknown-linux-gnueabihf/bin:/opt/armv7-rpi2-linux-gnueabihf/bin/:$PATH"
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
    echo "Unknown toolchain provided: $1. Choices: archarmv6|archarmv7|macv6|android|native|nativegcc6|mingw|clang4"
    exit 1
fi

checkresult mkdir -p ./build/ninja-${ARCH}
cd ./build/ninja-${ARCH}
pwd=`pwd`
checkresult cmake \
    -G Ninja \
    $platformopts \
    -DPKG_CONFIG_USE_CMAKE_PREFIX_PATH=ON \
    -DCMAKE_PREFIX_PATH=${PWD}/../local-${ARCH} \
    -DCMAKE_INSTALL_PREFIX=${pwd}/../local-${ARCH} \
    -DCMAKE_TOOLCHAIN_FILE=${pwd}/../../dependencies/toolchain-${ARCH}.make \
    -DOPENAL_INCLUDE_DIR=${pwd}/../local-${ARCH}/include \
    -DCMAKE_BUILD_TYPE=Release ../..
checkresult cmake --build .
