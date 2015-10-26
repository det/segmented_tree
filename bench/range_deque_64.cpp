#include "range.hpp"

#include <deque>

template <typename T>
using Container = std::deque<T>;

int main(int argc, char** argv) {
  return bench_range<Container, std::uint64_t>(argc, argv);
}
