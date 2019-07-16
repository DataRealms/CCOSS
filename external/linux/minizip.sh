#!/bin/bash

pushd $PWD/minizip-1.2
    make build
    cd build
    cmake -DCMAKE_INSTALL_PREFIX=$PWD/../install ../
    make
    make install
popd
