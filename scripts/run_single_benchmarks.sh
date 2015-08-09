#!/bin/bash

if [ "$#" -ne 6 ]; then
  echo "Expected 6 arguments" 1>&2
  exit 1
fi

function bench {
  for count in $3; do
    for benchmark in $4; do
      echo "Rusults for ${benchmark}<std::uint$5_t>, count: ${count}"
        for i in {0..20}; do
          $1/bench_single_${benchmark}_$5 $2/single.$5.${count}
        done | sort -k2 -k1,1n | uniq -f1
      echo
    done
  done  
}

bench $1 $2 "256 7936 246016" "$3" 8
bench $1 $2 "7626496" "$4" 8
bench $1 $2 "32 992 30752" "$5" 64
bench $1 $2 "953312" "$6" 64

