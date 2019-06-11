#!/bin/bash

src_file=uImage

declare -ia sizes=( $(mkimage -l "$src_file" |
  awk '/^ +Image [0-9]+/ { print $3 }') )
declare -i offset="68+4*${#sizes[@]}"
declare -i size

echo $sizes
echo $offset
echo $size

for i in "${!sizes[@]}"; do

  size=${sizes[$i]}

  echo "Unpacking image_$i"
  dd if="$src_file" of="image_$i" bs=1 skip="$offset" count="$size"

  # going to offset of next file while rounding to 4 byte multiple
  offset+=$(( size + (4 - size % 4) % 4 ))

done
