#!/bin/bash

if [ "$#" -ne 5 ]; then
  echo "Expected 5 arguments" 1>&2
  exit 1
fi

function bench {
  for benchmark in $2; do
    echo "Rusults for ${benchmark}<std::uint$3_t>, count: $4, size: $5"
      for i in {0..10}; do
        $1/bench_range_${benchmark}_$3 $4 $5 $6 $7
      done | sort -k2 -k1,1n | uniq -f1
    echo
  done
}

bench "$1" "$2" 8 1 7626496 1319121800 13937110459523125406
bench "$1" "$2" 8 31 246016 3037209825 7094631787510567342
bench "$1" "$2" 8 961 7936 2984486723 3526507821286439548
bench "$1" "$3" 8 29791 256 197924070 9499073426478457076

bench "$1" "$4" 64 1 953312 235951511 7803621008785366632
bench "$1" "$4" 64 31 30752 1082972474 11846815057285548515
bench "$1" "$4" 64 961 992 5659033 14482810490810820797
bench "$1" "$5" 64 29791 32 3727649439 10804193997107502541

