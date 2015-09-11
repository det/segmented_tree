#!/bin/bash

if [ "$#" -ne 5 ]; then
  echo "Expected 5 arguments" 1>&2
  exit 1
fi

function bench {
  for benchmark in $2; do
    echo "Rusults for ${benchmark}<std::uint$3_t>, count: $4"
      for i in {0..10}; do
        $1/bench_single_${benchmark}_$3 $4 $5 $6
      done | sort -k2 -k1,1n | uniq -f1
    echo
  done
}

bench "$1" "$2" 8 256 2107779313 15865477950454414828
bench "$1" "$2" 8 7936 976634119 3238950223561105499
bench "$1" "$2" 8 246016 3515491141 5644467892570804156
bench "$1" "$3" 8 7626496 1106530671 6965094249249093704

bench "$1" "$4" 64 32 2397254571 4723602420748635361
bench "$1" "$4" 64 992 463092544 12966777589746855639
bench "$1" "$4" 64 30752 430452927 751509891372566603
bench "$1" "$5" 64 953312 3109453262 10176667110359292238

