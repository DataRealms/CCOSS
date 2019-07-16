#!/bin/bash
./zlib.sh
./libpng.sh
./libogg.sh
./libvorbis.sh
./SDL.sh
./SDL_mixer.sh
./sndio.sh
./allegro.sh
./lua.sh
./minizip.sh

ZLIB_DIR=$PWD/zlib-1.2.11
PNG_DIR=$PWD/libpng-1.6.37
OGG_DIR=$PWD/libogg-1.3.3
VORBIS_DIR=$PWD/libvorbis-1.3.6
SDL_DIR=$PWD/SDL2-2.0.9
SDL_MIXER_DIR=$PWD/SDL2_mixer-2.0.4
SNDIO_DIR=$PWD/sndio-1.5.0/libsndio
ALLEGRO_DIR=$PWD/allegro5-4.4.3.1
LUA_DIR=$PWD/lua-5.1.5
MINIZIP_DIR=$PWD/minizip-1.2

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
    cp $SDL_MIXER_DIR/install/lib/libSDL2_mixer.a .
    cp -r $SDL_MIXER_DIR/install/include .
    cp $LUA_DIR/install/lib/liblua.a .
    cp -r $LUA_DIR/install/include .
    cp $MINIZIP_DIR/install/lib/libaes.a .
    cp $MINIZIP_DIR/install/lib/libminizip.a .
    cp -r $MINIZIP_DIR/install/include .
    cp $SNDIO_DIR/libsndio.a .
    cp -t $PWD/../../../libs $ALLEGRO_DIR/install/lib/*
popd
