#!/bin/bash

pushd $PWD/allegro5-4.4.3.1
    mkdir build
    cd build
    cmake -DCMAKE_INSTALL_PREFIX=$PWD/../install ../
    make
    make install
popd
