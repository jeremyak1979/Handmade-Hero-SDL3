#!/bin/bash

mkdir -p ../build
pushd ../build
gcc -Wall -o handher -lSDL3 ../code/handmade_sdl.c -g 
popd
