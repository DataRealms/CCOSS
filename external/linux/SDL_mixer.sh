#!/bin/bash

pushd $PWD/SDL2_mixer-2.0.4
    ./configure --prefix=$PWD/install --enable-shared=no
    make
    make install
popd
