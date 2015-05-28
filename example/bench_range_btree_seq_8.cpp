#include "bench_range.hpp"
#include "btree_seq.hpp"
#include "random_data_range.hpp"

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <8 bit generated random data>\n";
    return 1;
  }

  random_data_range<std::uint8_t> data{argv[1]};
  bench_range<btree_seq<std::uint8_t>>(data);
}
