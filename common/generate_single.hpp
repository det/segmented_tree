#ifndef BENCH_GENERATE_SINGLE
#define BENCH_GENERATE_SINGLE

#include <boost/lexical_cast.hpp>
#include <iostream>
#include "single.hpp"

template <typename T>
int generate_single(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <count>\n";
    return EXIT_FAILURE;
  }

  std::random_device device;
  auto seed = device();
  auto count = boost::lexical_cast<std::size_t>(argv[1]);
  auto data = make_insertion_data_single<T>(count, seed);
  std::vector<T> container;
  insert_single(container, data);
  std::cout << seed << " " << make_checksum_unsigned(container) << "\n";
  return EXIT_SUCCESS;
}

#endif  // #ifndef BENCH_GENERATE_SINGLE
