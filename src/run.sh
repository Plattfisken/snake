#!/bin/bash
PROGRAM_NAME="$(./build.sh)";
ARGS=""
if [ -n "$PROGRAM_NAME" ]; then
    ../build/"$PROGRAM_NAME" $ARGS;
fi
