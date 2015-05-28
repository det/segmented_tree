#include "avl_array.hpp"
#include "bench_range.hpp"
#include "random_data_range.hpp"

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <64 bit generated random data>\n";
    return 1;
  }

  random_data_range<std::uint64_t> data{argv[1]};
  bench_range<mkr::avl_array<std::uint64_t>>(data);
}
