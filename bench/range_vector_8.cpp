#include <vector>
#include "range.hpp"

template <typename T>
using Container = std::vector<T>;

int main(int argc, char** argv) {
  return bench_range<Container, std::uint8_t>(argc, argv);
}
