#!/bin/bash
set -v

export CC=gcc # clang --> package currently broken in fedora 23
export CFLAGS="-g -fsanitize=address"
./autogen.sh
./configure
cat config.log
make clean
export VERBOSE=1
make check
