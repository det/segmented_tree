#include "single.hpp"

#include <vector>

template <typename T>
using Container = std::vector<T>;

int main(int argc, char** argv) {
  return bench_single<Container, std::uint8_t>(argc, argv);
}
