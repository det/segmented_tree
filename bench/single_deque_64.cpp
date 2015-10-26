#include "single.hpp"

#include <deque>

template <typename T>
using Container = std::deque<T>;

int main(int argc, char** argv) {
  return bench_single<Container, std::uint64_t>(argc, argv);
}
