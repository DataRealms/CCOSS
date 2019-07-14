#!/bin/bash

pushd $PWD/zlib-1.2.11
    ./configure --static --prefix=$PWD/install
    make
    make install
popd
