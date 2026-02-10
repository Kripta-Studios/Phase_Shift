#!/bin/sh

set -xe

gcc -O3 -Wall -Wextra -std=c99 -o eepers.exe eepers.c -lraylib -lopengl32 -lgdi32 -lwinmm
./eepers.exe
