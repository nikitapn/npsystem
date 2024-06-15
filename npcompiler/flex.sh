#!/bin/bash

cd "$(dirname "$0")"

flex -t src/lang.l \
  | sed s/"#if defined (__STDC_VERSION__) && __STDC_VERSION__ >= 199901L"/"#if 1"/ \
  > src/generated/lex.yy.cc
