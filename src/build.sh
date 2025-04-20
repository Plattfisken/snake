#!/bin/bash
BUILD_MODE=$1

PROGRAM_NAME="Snake";
SOURCE_FILE_NAME="snake.c";

LIBS=""
# for raylib:
LIBS="$(pkg-config --libs --cflags raylib)"

# for sdl3:
# LIBS="$(pkg-config --libs --cflags sdl3)"

# set environment variables to help when debugging. stole from https://nullprogram.com/blog/2023/04/29/
export ASAN_OPTIONS=abort_on_error=1:halt_on_error=1
export UBSAN_OPTIONS=abort_on_error=1:halt_on_error=1

# Unused function and unused parameter warnings are turned off in debug mode but not in release mode.
# I generally don't want unused functions in the end but they are common during development and i wanna be able to build with them

# Release mode
if [ "$BUILD_MODE" = "--release" ]; then
    echo "Building $PROGRAM_NAME in release mode"
    FLAGS="-O3 -Wall -Wextra -Werror"
# Debug mode (default)
else
    WARNINGS="-Wall -Wextra -Werror -Wdouble-promotion -Wno-unused-function -Wno-unused-parameter"
    FLAGS="-g3 -O0 $WARNINGS -fsanitize=address,undefined -fsanitize-trap"
fi

mkdir -p ../build;
clang $SOURCE_FILE_NAME $LIBS $FLAGS -o ../build/"$PROGRAM_NAME" && echo "$PROGRAM_NAME"; # echo program name so it can be used by run.sh
