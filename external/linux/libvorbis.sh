#!/bin/bash

pushd $PWD/libvorbis-1.3.6
    export CFLAGS="-g"
    ./configure --prefix=$PWD/install --enable-shared=no --with-ogg=$PWD/../libogg-1.3.3/install
    make
    make install
popd
