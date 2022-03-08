#!/bin/bash

if [ $# -eq 0 ]; then
  echo "No arguments supplied"
  exit -1
fi

if [ ! -f "$1" ]; then
  echo "$1 does not exist."
  exit -1
fi

for size in 19 24 32 48 128 256; do
  inkscape -z -e $size.png -w $size -h $size $1 >/dev/null 2>/dev/null
done

convert 19.png 24.png 32.png 48.png 128.png 256.png ../npsystem/res/`basename $1 .svg`.ico

[ $? -eq 0 ] && exit 0  || exit 1

