#!/bin/bash

pushd $PWD/lua-5.1.5
    make linux
    make install
popd
