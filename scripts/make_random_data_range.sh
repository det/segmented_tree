#!/bin/bash

if [ "$#" -ne 2 ]; then
    echo "Usage $0 <build dir> <random data dir>" 1>&2
	exit 1
fi

for size in 256 7936 246016 7626496; do
  count=7626496
  let "count /= size"
  $1/gen_random_data_range_8 ${count} ${size} $2/range.8.${size}
done

for size in 32 992 30752 953312; do
  count=953312
  let "count /= ${size}"
  $1/gen_random_data_range_64 ${count} ${size} $2/range.64.${size}
done

