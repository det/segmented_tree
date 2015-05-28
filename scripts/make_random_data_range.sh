#!/bin/bash

if [ "$#" -ne 2 ]; then
    echo "Usage $0 <build dir> <random data dir>" 1>&2
	exit 1
fi

for size in 256 7936 246016 7626496; do
  x=7626496
  let "x /= ${size}"
  $1/example/gen_random_data_range_8 ${x} ${size} $2/range.8.${size}
done

for size in 32 992 30752 953312; do
  x=953312
  let "x /= ${size}"
  $1/example/gen_random_data_range_64 ${x} ${size} $2/range.64.${size}
done

