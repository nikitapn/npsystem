#!/bin/bash
#
#

set -ex

cmake -B build/linux -S . $*
cmake --build build/linux -j 4
