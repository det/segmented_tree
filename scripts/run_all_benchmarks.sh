#!/bin/bash

if [ "$#" -ne 2 ]; then
    echo "Usage $0 <build dir> <random data dir>" 1>&2
	exit 1
fi

for bits in 8 64; do
	for size in 1 2 4 8 16 32 64 128 256 512 1024 2048 4096 8192 16384 32768 65536 131072 262144 524288; do
		for benchmark in vector deque bpt_sequence btree_seq segment_tree; do
			echo "Rusults for ${benchmark}<std::uint${bits}_t> x ${size}"
			for i in {0..20}; do $1/example/bench_${benchmark}_${bits} $2/data.${bits}.${size}; done | sort -k2 -k1,1n | uniq -f1
			echo
		done
	done
done
