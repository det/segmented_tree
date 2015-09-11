#include "bench_common.hpp"
#include "random_data_range.hpp"

#include <cstdlib>

template <typename Container, typename T>
void erase_values_range(Container &container,
                        random_insert_data<T> const &data) {
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

template <template <typename T> class Container, typename T>
int bench_range(int argc, char **argv) {
  if (argc != 5) {
    std::cerr << "Usage: " << argv[0] << "<count> <size> <seed> <checksum>\n";
    return EXIT_FAILURE;
  }

  auto count = boost::lexical_cast<std::size_t>(argv[1]);
  auto size = boost::lexical_cast<std::size_t>(argv[2]);
  auto seed = boost::lexical_cast<std::uint32_t>(argv[3]);
  auto checksum = boost::lexical_cast<std::size_t>(argv[4]);

  auto data = make_insert_data_range<T>(count, size, seed);
  Container<T> container;
  bench("insert values", [&] { insert_values_range(container, data); });
  std::vector<T> inserted{container.begin(), container.end()};
  verify(checksum, make_checksum(inserted));
  bench_iterator(container, inserted);
  bench("erase values", [&] { erase_values_range(container, data); });
  verify(std::size_t{1}, container.size());
  verify(data.ordered[0], container[0]);
  return EXIT_SUCCESS;
}
