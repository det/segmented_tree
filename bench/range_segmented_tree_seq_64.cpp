#include "range.hpp"
#include "boost/segmented_tree_seq.hpp"

template <typename T>
using Container = boost::segmented_tree_seq<T>;

int main(int argc, char** argv) {
  return bench_range<Container, std::uint64_t>(argc, argv);
}
