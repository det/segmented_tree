#ifndef BENCH_SINGLE
#define BENCH_SINGLE

#include "iterator.hpp"
#include "../common/single.hpp"

#include <cstdlib>
#include <limits>
#include <boost/lexical_cast.hpp>

template <template <typename T> class Container, typename T>
int bench_single(int argc, char **argv) {
  if (argc != 4) {
    std::cerr << "Usage: " << argv[0] << " <count> <seed> <checksum>\n";
    return EXIT_FAILURE;
  }

  auto count = boost::lexical_cast<std::size_t>(argv[1]);
  auto seed = boost::lexical_cast<std::uint32_t>(argv[2]);
  auto checksum = boost::lexical_cast<std::uint64_t>(argv[3]);

  auto data = make_insertion_data_single<T>(count, seed);
  Container<T> container;
  bench("Insert values", [&] { insert_single(container, data); });
  std::vector<T> inserted{container.begin(), container.end()};
  verify(checksum, make_checksum_unsigned(inserted));
  bench_iterator(container, inserted);
  bench("Erase values", [&] { erase_single(container, data); });
  verify(container.size(), std::size_t{1});
  verify(container[0], data.ordered[0]);
  return EXIT_SUCCESS;
}

#endif  // #ifndef BENCH_SINGLE
