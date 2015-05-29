#!/bin/bash

if [ "$#" -ne 2 ]; then
    echo "Usage $0 <build dir> <random data dir>" 1>&2
	exit 1
fi

for count in 256 7936 246016 7626496; do
	$1/example/gen_random_data_single_8 ${count} $2/single.8.${count}
done

for count in 32 992 30752 953312; do
	$1/example/gen_random_data_single_64 ${count} $2/single.64.${count}
done

