#!/bin/bash

if [ "$#" -ne 6 ]; then
    echo "Expected 6 arguments" 1>&2
  exit 1
fi

function bench {
  for size in $3; do
    for benchmark in $4; do
      count=$6
      let "count /= ${size}"
      echo "Rusults for ${benchmark}<std::uint$5_t>, count: ${count}, size: ${size}"
        for i in {0..20}; do
          $1/bench_range_${benchmark}_$5 $2/range.$5.${size}
        done | sort -k2 -k1,1n | uniq -f1
      echo
    done
  done  
}

bench $1 $2 "7626496 246016 7936" "$3" 8 7626496
bench $1 $2 "256" "$4" 8 7626496
bench $1 $2 "953312 30752 992" "$5" 64 953312
bench $1 $2 "32" "$6" 64 953312

