#include "random_data_single.hpp"

#include <string>

int main(int argc, char** argv) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <count> <output path>\n";
    return 1;
  }

  std::size_t size = std::stoull(argv[1]);
  random_data_single<std::uint64_t> data{size};
  data.save(argv[2]);
}
