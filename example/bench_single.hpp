#include "bench_common.hpp"
#include "random_data_single.hpp"

#include <cstdlib>
#include <limits>

#include <boost/lexical_cast.hpp>

template <typename Container, typename T>
void erase_values_single(Container &container,
                         random_insert_data<T> const &data) {
  auto count = data.indexes.size();
  reserve(container, count);
  while (count != 1) {
    --count;
    container.erase(nth(container, data.indexes[count]));
  }
}

template <template <typename T> class Container, typename T>
int bench_single(int argc, char **argv) {
  if (argc != 4) {
    std::cerr << "Usage: " << argv[0] << " <count> <seed> <checksum>\n";
    return EXIT_FAILURE;
  }

  auto count = boost::lexical_cast<std::size_t>(argv[1]);
  auto seed = boost::lexical_cast<std::uint32_t>(argv[2]);
  auto checksum = boost::lexical_cast<std::size_t>(argv[3]);

  auto data = make_insert_data_single<T>(count, seed);
  Container<T> container;
  bench("insert values", [&] { insert_values_single(container, data); });
  std::vector<T> inserted{container.begin(), container.end()};
  verify(checksum, make_checksum(inserted));
  bench_iterator(container, inserted);
  bench("erase values", [&] { erase_values_single(container, data); });
  verify(container.size(), std::size_t{1});
  verify(container[0], data.ordered[0]);
  return EXIT_SUCCESS;
}
