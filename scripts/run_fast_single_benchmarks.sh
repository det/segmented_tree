#!/bin/bash

if [ "$#" -ne 2 ]; then
    echo "Usage $0 <build dir> <random data dir>" 1>&2
	exit 1
fi

for count in 256 7936 246016 7626496; do
	for benchmark in btree_seq segmented_tree_seq; do
		echo "Rusults for ${benchmark}<std::uint8_t>, count: ${count}"
		for i in {0..20}; do $1/bench_single_${benchmark}_8 $2/single.8.${count}; done | sort -k2 -k1,1n | uniq -f1
		echo
	done
done

for count in 32 992 30752 953312; do
	for benchmark in btree_seq segmented_tree_seq; do
		echo "Rusults for ${benchmark}<std::uint64_t>, count: ${count}"
		for i in {0..20}; do $1/bench_single_${benchmark}_64 $2/single.64.${count}; done | sort -k2 -k1,1n | uniq -f1
		echo
	done
done

