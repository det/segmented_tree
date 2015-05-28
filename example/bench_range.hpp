#include "bench_common.hpp"
#include "random_data_range.hpp"

template <typename Container, typename T>
void insert_values_range(Container &container,
                         random_data_range<T> const &data) {
  auto count = data.get_count();
  auto size = data.get_size();
  auto &indexes = data.get_indexes();
  auto &ordered = data.get_ordered();
  reserve(container, count);
  auto first = ordered.begin();
  auto last = first;
  for (std::size_t i = 0; i != count; ++i) {
    last += size;
    container.insert(position(container, indexes[i]), first, last);
    first += size;
  }
}

template <typename Container, typename T>
void erase_values_range(Container &container,
                        random_data_range<T> const &data) {
  auto count = data.get_count();
  auto size = data.get_size();
  auto &indexes = data.get_indexes();

  while (count != 1) {
    --count;
    auto first = position(container, indexes[count]);
    auto last = first + size;
    container.erase(first, last);
  }
}

template <typename Container, typename T>
void bench_range(random_data_range<T> const &data) {
  std::cout << std::fixed;
  Container container;

  bench("insert values", insert_values_range<Container, T>, container, data);
  bench_iterator(container, data.get_inserted());
  bench("erase values", erase_values_range<Container, T>, container, data);
  for (std::size_t i = 0; i != data.get_size(); ++i)
    verify(data.get_ordered()[i], container[i]);
}
