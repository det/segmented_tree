#include "avl_array/avl_array/src/avl_array.hpp"
#include "range.hpp"

template <typename T>
using Container = mkr::avl_array<T>;

int main(int argc, char** argv) {
  return bench_range<Container, std::uint8_t>(argc, argv);
}
