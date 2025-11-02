#!/bin/bash

set -ex

# Ninja does not produce colored output issue: https://stackoverflow.com/a/79399221

export CLICOLOR_FORCE=1

mkdir -p build/linux
cmake -G "Ninja" -B build/linux -S . -DCMAKE_CXX_FLAGS=-fdiagnostics-color=always -DCMAKE_COLOR_DIAGNOSTICS=ON $*
cmake --build build/linux -j$(nproc)
