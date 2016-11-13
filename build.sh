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

checkresult mkdir -p ./build/ninja
cd ./build/ninja
PWD=`pwd`
checkresult cmake -G Ninja -DPKG_CONFIG_USE_CMAKE_PREFIX_PATH=ON -DCMAKE_PREFIX_PATH=${PWD}/../local ../..
checkresult ninja
