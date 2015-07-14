#include "bench_single.hpp"
#include "array/trunk/btree_seq.h"
#include "random_data_single.hpp"

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <8 bit generated random data>\n";
    return 1;
  }

  random_data_single<std::uint8_t> data{argv[1]};
  bench_single<btree_seq<std::uint8_t>>(data);
}
