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

checkresult rm -rf ./build
cd dependencies
checkresult ./build.sh native
cd ..
checkresult mkdir -p ./build/ninja
cd ./build/ninja
checkresult cmake -G Ninja -DLOCAL_DEPS=ON ../..
checkresult ninja
