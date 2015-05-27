#include "avl_array.hpp"
#include "bench_single.hpp"
#include "random_data_single.hpp"

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <8 bit generated random data>\n";
    return 1;
  }

  random_data_t<std::uint8_t> data{argv[1]};
  bench_container<mkr::avl_array<std::uint8_t>>(data);
}
