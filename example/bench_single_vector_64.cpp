#include "bench_single.hpp"
#include "random_data_single.hpp"

#include <vector>

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <64 bit generated random data>\n";
    return 1;
  }

  random_data_single<std::uint64_t> data{argv[1]};
  bench_single<std::vector<std::uint64_t>>(data);
}