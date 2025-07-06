#!/bin/bash

set -e

if [ $# -eq 0 ]; then
  echo "No arguments supplied"
  exit -1
fi

INPUT_FILE="$1"
OUTPUT_DIR="$2"

if [ ! -f "$INPUT_FILE" ]; then
  echo "$1 does not exist."
  exit -1
fi

if [[ "$INPUT_FILE" != *.svg ]]; then
  echo "Input file must be a .svg file."
  exit -1
fi

if [ -z "$OUTPUT_DIR" ]; then
  OUTPUT_DIR="."
fi

TMP_DIR=$(mktemp -d)
trap 'rm -rf "$TMP_DIR"' EXIT

for size in 19 24 32 48 128 256; do
  inkscape \
    --export-area-page \
    --export-png-color-mode=RGBA_8 \
    --export-filename="$1_$size.png" \
    --export-width=$size --export-height=$size $1 \
    -o "$TMP_DIR/$size.png" >/dev/null 2>/dev/null

  #inkscape -z -e $size.png -w $size -h $size $1 >/dev/null 2>/dev/null
done

magick convert $TMP_DIR/19.png $TMP_DIR/24.png $TMP_DIR/32.png \
  $TMP_DIR/48.png $TMP_DIR/128.png $TMP_DIR/256.png \
  `basename $1 .svg`.ico

cp "$TMP_DIR/256.png" "$OUTPUT_DIR/`basename $1 .svg`.png"
