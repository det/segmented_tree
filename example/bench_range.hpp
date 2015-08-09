#include "bench_common.hpp"
#include "random_data_range.hpp"

#include <cstdlib>

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
    last += static_cast<std::ptrdiff_t>(size);
    container.insert(nth(container, indexes[i]), first, last);
    first += static_cast<std::ptrdiff_t>(size);
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
    auto first = nth(container, indexes[count]);
    auto last = first + static_cast<std::ptrdiff_t>(size);
    container.erase(first, last);
  }
  container.erase(nth(container, 1), container.end());
}

template <template <typename T> class Container, typename T>
int bench_range(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <" << std::numeric_limits<T>::digits
              << "-bit generated range random data>\n";
    return EXIT_FAILURE;
  }

  std::cout << std::fixed;
  random_data_range<T> data{argv[1]};
  Container<T> container;

  bench("insert values", [&] { insert_values_range(container, data); });
  bench_iterator(container, data.get_inserted());
  bench("erase values", [&] { erase_values_range(container, data); });
  verify(std::size_t{1}, container.size());
  verify(data.get_ordered()[0], container[0]);
  return EXIT_SUCCESS;
}
