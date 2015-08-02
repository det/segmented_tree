#!/bin/bash

if [ "$#" -ne 2 ]; then
    echo "Usage $0 <build dir> <random data dir>" 1>&2
	exit 1
fi

for size in 256 7936 246016 7626496; do
	for benchmark in btree_seq segmented_tree_seq; do
    count=7626496
    let "count /= ${size}"
		echo "Rusults for ${benchmark}<std::uint8_t>, count: ${count}, size: ${size}"
		for i in {0..20}; do $1/bench_range_${benchmark}_8 $2/range.8.${size}; done | sort -k2 -k1,1n | uniq -f1
		echo
	done
done

for size in 32 992 30752 953312; do
	for benchmark in btree_seq segmented_tree_seq; do
    count=953312
    let "count /= ${size}"
		echo "Rusults for ${benchmark}<std::uint64_t>, count: ${count}, size: ${size}"
		for i in {0..20}; do $1/bench_range_${benchmark}_64 $2/range.64.${size}; done | sort -k2 -k1,1n | uniq -f1
		echo
	done
done

