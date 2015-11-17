#include "range.hpp"

#include <vector>

template <typename T>
using Container = std::vector<T>;

int main(int argc, char** argv) {
  return bench_range<Container, std::uint64_t>(argc, argv);
}