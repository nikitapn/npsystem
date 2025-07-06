#!/bin/bash

if [ $# -eq 0 ]; then
  echo "No arguments supplied"
  exit -1
fi

if [ ! -f "$1" ]; then
  echo "$1 does not exist."
  exit -1
fi

# inkscape -z -e $1.png -w 64 -h 56 $1 >/dev/null 2>/dev/null
convert $1 -resize 64x56 -define BMP3:alpha=true -background none -transparent white BMP3:../npsystem/res/blocks/`basename $1 .svg`.bmp

[ $? -eq 0 ] && exit 0  || exit 1

