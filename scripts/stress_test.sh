#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Usage $0 <build dir>" 1>&2
	exit 1
fi

data_dir=`mktemp -d`

while true; do
	for bits in 8 64; do
			random=$RANDOM
			let "random += 1"
			$1/example/gen_random_data_${bits} $RANDOM ${data_dir}/data
			for benchmark in segment_tree; do
				$1/example/bench_${benchmark}_${bits} ${data_dir}/data > /dev/null
			done
		done
	done
done