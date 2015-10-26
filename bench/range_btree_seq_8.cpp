#include "range.hpp"
#include "array/trunk/btree_seq.h"

template <typename T>
using Container = btree_seq<T>;

int main(int argc, char** argv) {
  return bench_range<Container, std::uint8_t>(argc, argv);
}
