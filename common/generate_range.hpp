#ifndef BENCH_GENERATE_RANGE
#define BENCH_GENERATE_RANGE

#include <boost/lexical_cast.hpp>
#include <iostream>
#include "range.hpp"

template <typename T>
int generate_range(int argc, char **argv) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <count> <size>\n";
    return EXIT_FAILURE;
  }

  std::random_device device;
  auto seed = device();
  auto count = boost::lexical_cast<std::size_t>(argv[1]);
  auto size = boost::lexical_cast<std::size_t>(argv[2]);
  auto data = make_insertion_data_range<T>(count, size, seed);
  std::vector<T> container;
  insert_range(container, data);
  std::cout << seed << " " << make_checksum_unsigned(container) << "\n";
  return EXIT_SUCCESS;
}

#endif  // #ifndef BENCH_GENERATE_RANGE
