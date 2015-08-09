#pragma once

#include "random_data_common.hpp"

#include <array>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <vector>
#include <random>
#include <type_traits>

template <typename T>
class random_data_range {
  static_assert(std::is_trivial<T>::value, "T must be trivial");
  static_assert(std::is_integral<T>::value, "T must be integral");
  static_assert(std::is_unsigned<T>::value, "T must be unsigned");

 private:
  std::vector<std::size_t> indexes_;
  std::vector<T> ordered_;
  std::vector<T> inserted_;

 public:
  random_data_range(std::size_t count, std::size_t size) {
    std::random_device device;
    std::default_random_engine engine{device()};
    std::uniform_int_distribution<std::uintmax_t> dist{
        0, (std::numeric_limits<T>::max)()};
    indexes_.reserve(count);
    ordered_.reserve(count * size);
    inserted_.reserve(count * size);

    auto first = ordered_.end();
    auto last = first;
    for (std::size_t i = 0; i != count; ++i) {
      for (std::size_t j = 0; j != size; ++j)
        ordered_.push_back(static_cast<T>(dist(engine)));
      std::uniform_int_distribution<std::size_t> index_dist{0, i * size};
      std::size_t index = index_dist(engine);
      indexes_.push_back(index);
      auto pos = inserted_.begin() + static_cast<std::ptrdiff_t>(index);
      last += static_cast<std::ptrdiff_t>(size);
      inserted_.insert(pos, first, last);
      first += static_cast<std::ptrdiff_t>(size);
    }
  }

  random_data_range(std::string const &path) {
    std::ifstream in{path, std::ios::binary};
    std::size_t count;
    std::size_t size;
    in.read(reinterpret_cast<char *>(&count), sizeof(std::size_t));
    in.read(reinterpret_cast<char *>(&size), sizeof(std::size_t));
    Read(in, indexes_, count);
    Read(in, ordered_, count * size);
    Read(in, inserted_, count * size);
  }

  void save(std::string const &path) {
    std::ofstream out{path, std::ios::binary};
    std::size_t count = get_count();
    std::size_t size = get_size();
    out.write(reinterpret_cast<char *>(&count), sizeof(std::size_t));
    out.write(reinterpret_cast<char *>(&size), sizeof(std::size_t));
    Write(out, indexes_);
    Write(out, ordered_);
    Write(out, inserted_);
  }

  std::size_t get_count() const { return indexes_.size(); }
  std::size_t get_size() const { return ordered_.size() / get_count(); }
  std::vector<std::size_t> const &get_indexes() const { return indexes_; }
  std::vector<T> const &get_inserted() const { return inserted_; }
  std::vector<T> const &get_ordered() const { return ordered_; }
};

template <typename T>
int generate_range(int argc, char **argv) {
  if (argc != 4) {
    std::cerr << "Usage: " << argv[0] << " <count> <size> <output path>\n";
    return EXIT_FAILURE;
  }

  std::size_t count = std::strtoul(argv[1], nullptr, 10);
  std::size_t size = std::strtoul(argv[2], nullptr, 10);
  random_data_range<T> data{count, size};
  data.save(argv[3]);
  return EXIT_SUCCESS;
}
