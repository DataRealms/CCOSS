#!/bin/bash
./zlib.sh
./libpng.sh
./libogg.sh
./libvorbis.sh
./SDL.sh
./sndio.sh
./allegro.sh

ZLIB_DIR=$PWD/zlib-1.2.11
PNG_DIR=$PWD/libpng-1.6.37
OGG_DIR=$PWD/libogg-1.3.3
VORBIS_DIR=$PWD/libvorbis-1.3.6
SDL_DIR=$PWD/SDL2-2.0.9
SNDIO_DIR=$PWD/sndio-1.5.0/libsndio
ALLEGRO_DIR=$PWD/allegro5-4.4.3.1

mkdir $PWD/prepared
mkdir $PWD/prepared/include
pushd $PWD/prepared
    cp $ZLIB_DIR/install/lib/libz.a .
    cp -r $ZLIB_DIR/install/include .
    cp $PNG_DIR/install/lib/libpng16.a .
    cp -r $PNG_DIR/install/include .
    cp $OGG_DIR/install/lib/libogg.a .
    cp -r $OGG_DIR/install/include .
    cp $VORBIS_DIR/install/lib/libvorbis.a .
    cp $VORBIS_DIR/install/lib/libvorbisenc.a .
    cp $VORBIS_DIR/install/lib/libvorbisfile.a .
    cp -r $VORBIS_DIR/install/include .
    cp $SDL_DIR/install/lib/libSDL2.a .
    cp -r $SDL_DIR/install/include .
    cp $SNDIO_DIR/libsndio.a .
    cp -t $PWD/../../../libs $ALLEGRO_DIR/install/lib/*
popd
