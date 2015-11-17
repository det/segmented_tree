#ifndef BENCH_RANGE
#define BENCH_RANGE

#include "iterator.hpp"
#include "../common/range.hpp"

#include <cstdlib>
#include <limits>
#include <boost/lexical_cast.hpp>

template <template <typename T> class Container, typename T>
int bench_range(int argc, char **argv) {
  if (argc != 5) {
    std::cerr << "Usage: " << argv[0] << "<count> <size> <seed> <checksum>\n";
    return EXIT_FAILURE;
  }

  auto count = boost::lexical_cast<std::size_t>(argv[1]);
  auto size = boost::lexical_cast<std::size_t>(argv[2]);
  auto seed = boost::lexical_cast<std::uint32_t>(argv[3]);
  auto checksum = boost::lexical_cast<std::uint64_t>(argv[4]);

  auto data = make_insertion_data_range<T>(count, size, seed);
  Container<T> container;
  bench("Insert values", [&] { insert_range(container, data); });
  std::vector<T> inserted{container.begin(), container.end()};
  verify(checksum, make_checksum_unsigned(inserted));
  bench_iterator(container, inserted);
  bench("Erase values", [&] { erase_range(container, data); });
  verify(std::size_t{1}, container.size());
  verify(data.ordered[0], container[0]);
  return EXIT_SUCCESS;
}

#endif  // #ifndef BENCH_RANGE
