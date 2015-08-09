#!/bin/bash

if [ "$#" -ne 2 ]; then
  echo "Usage $0 <build dir> <random data dir>" 1>&2
  exit 1
fi

script=`dirname $0`/run_single_benchmarks.sh
$script "$1" "$2" \
        "vector deque avl_array btree_seq segmented_tree_seq" \
        "avl_array btree_seq segmented_tree_seq" \
        "vector deque avl_array btree_seq segmented_tree_seq" \
        "avl_array btree_seq segmented_tree_seq"

