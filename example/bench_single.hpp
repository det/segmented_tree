#include "bench_common.hpp"
#include "random_data_single.hpp"

template <typename Container, typename T>
void insert_values_single(Container &container,
                          random_data_single<T> const &data) {
  auto count = data.get_count();
  auto &indexes = data.get_indexes();
  auto &ordered = data.get_ordered();
  reserve(container, count);
  for (std::size_t i = 0; i != count; ++i) {
    container.insert(position(container, indexes[i]), ordered[i]);
  }
}

template <typename Container, typename T>
void erase_values_single(Container &container,
                         random_data_single<T> const &data) {
  auto count = data.get_count();
  auto indexes = data.get_indexes();
  reserve(container, count);
  while (count != 1) {
    --count;
    container.erase(position(container, indexes[count]));
  }
}

template <typename Container, typename T>
void bench_single(random_data_single<T> const &data) {
  std::cout << std::fixed;
  Container container;

  bench("insert values", insert_values_single<Container, T>, container, data);
  bench_iterator(container, data.get_inserted());
  bench("erase values", erase_values_single<Container, T>, container, data);
  verify(std::size_t{1}, container.size());
  verify(data.get_ordered()[0], container[0]);
}
