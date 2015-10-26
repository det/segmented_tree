#ifndef COMMON_SINGLE
#define COMMON_SINGLE

#include "common.hpp"

template <typename Container, typename T>
void insert_single(Container &container, insertion_data<T> const &data) {
  auto count = data.indexes.size();
  reserve(container, count);
  for (std::size_t i = 0; i != count; ++i) {
    container.insert(nth(container, data.indexes[i]), data.ordered[i]);
  }
}

template <typename Container, typename T>
void erase_single(Container &container, insertion_data<T> const &data) {
  auto count = data.indexes.size();
  reserve(container, count);
  while (count != 1) {
    --count;
    container.erase(nth(container, data.indexes[count]));
  }
}

template <typename T>
insertion_data<T> make_insertion_data_single(std::size_t count,
                                             std::uint32_t seed) {
  seed_seq seq{seed};
  random_engine engine{seq};
  random_bits_engine<T> bits_engine{seq};
  insertion_data<T> data;
  data.indexes.reserve(count);
  data.ordered.reserve(count);

  for (std::size_t i = 0; i != count; ++i) {
    data.indexes.push_back(bounded_rand(engine, i + 1));
    data.ordered.push_back(bits_engine());
  }

  return data;
}

#endif  // #ifndef COMMON_SINGLE
