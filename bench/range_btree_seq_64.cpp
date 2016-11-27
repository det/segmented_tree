#include "array/trunk/btree_seq.h"
#include "range.hpp"

template <typename T>
using Container = btree_seq<T>;

int main(int argc, char** argv) {
  return bench_range<Container, std::uint64_t>(argc, argv);
}
