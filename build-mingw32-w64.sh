#!/bin/sh

set -xe

x86_64-w64-mingw32-windres eepers.rc -O coff -o eepers.res
x86_64-w64-mingw32-gcc -O3 -Wall -Wextra -std=c99 -o eepers.exe eepers.c eepers.res -lraylib -lopengl32 -lgdi32 -lwinmm -static
