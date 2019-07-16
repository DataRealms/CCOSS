#!/bin/bash

pushd $PWD/SDL2-2.0.9
    ./configure --prefix=$PWD/install --enable-shared=no
    make
    make install
popd
