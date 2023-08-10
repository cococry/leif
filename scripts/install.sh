#!/usr/bin/env bash

if [ ! -d "../bin" ]; then
    mkdir ../bin
fi
./build_deps.sh
echo "[INFO]: Building Leif (leif.c)"
./build_all.sh
