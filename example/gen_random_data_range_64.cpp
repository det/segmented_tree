#include "random_data_range.hpp"

#include <string>

int main(int argc, char** argv) {
  if (argc != 4) {
    std::cerr << "Usage: " << argv[0] << " <count> <size> <output path>\n";
    return 1;
  }

  std::size_t count = std::strtoul(argv[1], nullptr, 10);
  std::size_t size = std::strtoul(argv[2], nullptr, 10);
  random_data_range<std::uint64_t> data{count, size};
  data.save(argv[3]);
}
