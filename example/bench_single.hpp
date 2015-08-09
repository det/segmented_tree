#include "bench_common.hpp"
#include "random_data_single.hpp"

#include <cstdlib>
#include <limits>

template <typename Container, typename T>
void insert_values_single(Container &container,
                          random_data_single<T> const &data) {
  auto count = data.get_count();
  auto &indexes = data.get_indexes();
  auto &ordered = data.get_ordered();
  reserve(container, count);
  for (std::size_t i = 0; i != count; ++i) {
    container.insert(nth(container, indexes[i]), ordered[i]);
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
    container.erase(nth(container, indexes[count]));
  }
}

template <template <typename T> class Container, typename T>
int bench_single(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <" << std::numeric_limits<T>::digits
              << "-bit generated single random data>\n";
    return EXIT_FAILURE;
  }

  std::cout << std::fixed;
  random_data_single<T> data{argv[1]};
  Container<T> container;

  bench("insert values", [&] { insert_values_single(container, data); });
  bench_iterator(container, data.get_inserted());
  bench("erase values", [&] { erase_values_single(container, data); });
  verify(std::size_t{1}, container.size());
  verify(data.get_ordered()[0], container[0]);
  return EXIT_SUCCESS;
}
