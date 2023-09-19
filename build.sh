#!/bin/sh

set -xe

CFLAGS="-Wall -Wextra -ggdb"
LIBS="-lpthread"
FILES="-I./src src/*.c"

mkdir -p ./build/
clang $CFLAGS -o ./build/tuna $FILES $LIBS
