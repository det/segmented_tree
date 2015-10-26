#include "single.hpp"
#include "boost/segmented_tree_seq.hpp"

template <typename T>
using Container = boost::segmented_tree_seq<T>;

int main(int argc, char** argv) {
  return bench_single<Container, std::uint8_t>(argc, argv);
}
