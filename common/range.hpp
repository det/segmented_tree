#ifndef COMMON_RANGE
#define COMMON_RANGE

#include "common.hpp"

template <typename Container, typename T>
void insert_range(Container& container, insertion_data<T> const& data) {
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

template <typename Container, typename T>
void erase_range(Container& container, insertion_data<T> const& data) {
  auto count = data.indexes.size();
  auto size = data.ordered.size() / count;

  while (count != 1) {
    --count;
    auto first = nth(container, data.indexes[count]);
    auto last = first + static_cast<std::ptrdiff_t>(size);
    container.erase(first, last);
  }
  container.erase(nth(container, 1), container.end());
}

template <typename T>
insertion_data<T> make_insertion_data_range(std::size_t count, std::size_t size,
                                            std::uint32_t seed) {
  seed_seq seq{seed};
  random_engine engine{seq};
  random_bits_engine<T> bits_engine{seq};
  insertion_data<T> data;
  data.indexes.reserve(count);
  data.ordered.reserve(count * size);

  for (std::size_t i = 0; i != count; ++i) {
    for (std::size_t j = 0; j != size; ++j)
      data.ordered.push_back(bits_engine());
    data.indexes.push_back(bounded_rand(engine, i * size + 1));
  }

  return data;
}

#endif  // #ifndef COMMON_RANGE
