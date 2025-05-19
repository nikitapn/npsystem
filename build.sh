#!/bin/bash
#
#

cmake -B build/linux -S . $* && cmake --build build/linux -j 4
