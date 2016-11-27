#include "boost/segmented_tree/seq.hpp"
#include "range.hpp"

template <typename T>
using Container = boost::segmented_tree::seq<T>;

int main(int argc, char** argv) {
  return bench_range<Container, std::uint8_t>(argc, argv);
}
