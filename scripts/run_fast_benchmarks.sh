#!/bin/bash

if [ "$#" -ne 2 ]; then
    echo "Usage $0 <build dir> <random data dir>" 1>&2
	exit 1
fi

for bits in 8 64; do
	for size in 524288; do
		for benchmark in btree_seq segment_tree; do
			echo "Rusults for ${benchmark}<std::uint${bits}_t> x ${size}"
			for i in {0..20}; do $1/example/bench_${benchmark}_${bits} $2/data.${bits}.${size}; done | sort -k2 -k1,1n | uniq -f1
			echo
		done
	done
done
