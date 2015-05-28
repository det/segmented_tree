#!/bin/bash

if [ "$#" -ne 2 ]; then
    echo "Usage $0 <build dir> <random data dir>" 1>&2
	exit 1
fi

for size in 256 7936 246016; do
	for benchmark in btree_seq segmented_tree_seq; do
		echo "Rusults for ${benchmark}<std::uint8_t> x ${size}"
		for i in {0..20}; do $1/example/bench_single_${benchmark}_8 $2/single.8.${size}; done | sort -k2 -k1,1n | uniq -f1
		echo
	done
done

for size in 32 992 30752; do
	for benchmark in btree_seq segmented_tree_seq; do
		echo "Rusults for ${benchmark}<std::uint64_t> x ${size}"
		for i in {0..20}; do $1/example/bench_single_${benchmark}_64 $2/single.64.${size}; done | sort -k2 -k1,1n | uniq -f1
		echo
	done
done

