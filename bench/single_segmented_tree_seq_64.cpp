#include "boost/segmented_tree/seq.hpp"
#include "single.hpp"

template <typename T>
using Container = boost::segmented_tree::seq<T>;

int main(int argc, char** argv) {
  return bench_single<Container, std::uint64_t>(argc, argv);
}
