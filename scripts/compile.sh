#!/bin/bash

premake5 gmake2

if [ "$1" = "release" ]; then
    make config=release
else
    make
fi
