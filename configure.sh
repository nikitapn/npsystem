#!/bin/bash

set -ex

# Ninja does not produce colored output issue: https://stackoverflow.com/a/79399221
# export CLICOLOR_FORCE=1

mkdir -p build/linux
# Use Unix Makefiles to avoid ninja color output issues and broken C++/Intellisense in VSCode
cmake -G "Unix Makefiles" -B build/linux -S . \
  -DCMAKE_BUILD_TYPE=Debug \
  $*
