#!/bin/bash

if [ "$#" -ne 2 ]; then
    echo "Usage $0 <build dir> <random data dir>" 1>&2
	exit 1
fi

for count in 256 7936 246016 7626496; do
  size=7626496
  let "size /= ${count}"
  $1/example/gen_random_data_range_8 ${size} ${count} $2/range.8.${count}
done

for count in 32 992 30752 953312; do
  size=953312
  let "size /= ${count}"
  $1/example/gen_random_data_range_64 ${size} ${count} $2/range.64.${count}
done

