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
			$1/gen_random_data_single_${bits} $RANDOM ${data_dir}/data
			for benchmark in segmented_tree_seq; do
				$1/bench_single_${benchmark}_${bits} ${data_dir}/data > /dev/null
			done
		done
	done
done
