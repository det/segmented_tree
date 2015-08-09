#include "bench_single.hpp"
#include "stl_ext_adv_review/container/bpt_sequence.hpp"

template <typename T>
using Container = std_ext_adv::sequence<T>;

int main(int argc, char** argv) {
  return bench_single<Container, std::uint8_t>(argc, argv);
}
