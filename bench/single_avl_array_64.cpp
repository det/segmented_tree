#include "avl_array/avl_array/src/avl_array.hpp"
#include "single.hpp"

template <typename T>
using Container = mkr::avl_array<T>;

int main(int argc, char** argv) {
  return bench_single<Container, std::uint64_t>(argc, argv);
}
