#!/bin/bash

pushd $PWD/libpng-1.6.37
    ./configure --prefix=$PWD/install --enable-shared=no --with-sysroot=$PWD/../zlib-1.2.11/install
    make
    make install
popd
