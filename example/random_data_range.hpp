#pragma once

#include "random_data_common.hpp"

template <typename T>
random_insert_data<T> make_insert_data_range(std::size_t count,
                                             std::size_t size,
                                             std::uint32_t seed) {
  seed_seq seq{seed};
  random_engine engine{seq};
  random_bits_engine<T> bits_engine{seq};
  random_insert_data<T> data;
  data.indexes.reserve(count);
  data.ordered.reserve(count * size);

  for (std::size_t i = 0; i != count; ++i) {
    for (std::size_t j = 0; j != size; ++j)
      data.ordered.push_back(bits_engine());
    data.indexes.push_back(bounded_rand(engine, i * size + 1));
  }

  return data;
}

template <typename Container, typename T>
void insert_values_range(Container &container,
                         random_insert_data<T> const &data) {
  auto count = data.indexes.size();
  auto size = data.ordered.size() / count;
  reserve(container, count);
  auto first = data.ordered.data();
  auto last = first;
  for (std::size_t i = 0; i != count; ++i) {
    last += size;
    container.insert(nth(container, data.indexes[i]), first, last);
    first += size;
  }
}

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
  auto data = make_insert_data_range<T>(count, size, seed);
  std::vector<T> container;
  insert_values_range(container, data);
  std::cout << seed << " " << make_checksum(container) << "\n";
  return EXIT_SUCCESS;
}
