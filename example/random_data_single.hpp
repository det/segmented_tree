#pragma once

#include "random_data_common.hpp"

template <typename T>
random_insert_data<T> make_insert_data_single(std::size_t count,
                                              std::uint32_t seed) {
  seed_seq seq{seed};
  random_engine engine{seq};
  random_bits_engine<T> bits_engine{seq};
  random_insert_data<T> data;
  data.indexes.reserve(count);
  data.ordered.reserve(count);

  for (std::size_t i = 0; i != count; ++i) {
    data.indexes.push_back(bounded_rand(engine, i + 1));
    data.ordered.push_back(bits_engine());
  }

  return data;
}

template <typename Container, typename T>
void insert_values_single(Container &container,
                          random_insert_data<T> const &data) {
  auto count = data.indexes.size();
  reserve(container, count);
  for (std::size_t i = 0; i != count; ++i) {
    container.insert(nth(container, data.indexes[i]), data.ordered[i]);
  }
}

template <typename T>
int generate_single(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <count>\n";
    return EXIT_FAILURE;
  }

  std::random_device device;
  auto seed = device();
  auto count = boost::lexical_cast<std::size_t>(argv[1]);
  auto data = make_insert_data_single<T>(count, seed);
  std::vector<T> container;
  insert_values_single(container, data);
  std::cout << seed << " " << make_checksum(container) << "\n";
  return EXIT_SUCCESS;
}
