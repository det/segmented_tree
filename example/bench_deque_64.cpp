#include "bench.hpp"
#include "random_data.hpp"

#include <deque>

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <64 bit generated random data>\n";
    return 1;
  }

  random_data_t<std::uint64_t> data{argv[1]};
  bench_container<std::deque<std::uint64_t>>(data);
}
