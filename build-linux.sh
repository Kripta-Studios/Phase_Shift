#!/bin/sh

set -xe

gcc -O3 -Wall -Wextra -std=c99 -o eepers-linux eepers.c -lraylib -lm -lpthread
./eepers-linux
