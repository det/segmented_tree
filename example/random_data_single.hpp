#pragma once

#include "random_data_common.hpp"

#include <array>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
#include <random>
#include <type_traits>

template <typename T>
class random_data_single {
  static_assert(std::is_trivial<T>::value, "T must be trivial");
  static_assert(std::is_integral<T>::value, "T must be integral");
  static_assert(std::is_unsigned<T>::value, "T must be unsigned");

 private:
  std::vector<std::size_t> indexes_;
  std::vector<T> ordered_;
  std::vector<T> inserted_;

 public:
  random_data_single(std::size_t count) {
    std::random_device device;
    std::default_random_engine engine{device()};
    std::uniform_int_distribution<std::uintmax_t> dist{
        0, (std::numeric_limits<T>::max)()};
    indexes_.reserve(count);
    ordered_.reserve(count);
    inserted_.reserve(count);

    for (std::size_t i = 0; i != count; ++i) {
      std::uniform_int_distribution<std::size_t> index_dist{0, i};
      std::size_t index = index_dist(engine);
      T data = static_cast<T>(dist(engine));
      indexes_.push_back(index);
      ordered_.push_back(data);
      inserted_.insert(inserted_.begin() + static_cast<std::ptrdiff_t>(index),
                       data);
    }
  }

  random_data_single(std::string const &path) {
    std::ifstream in{path, std::ios::binary};
    std::size_t count;
    in.read(reinterpret_cast<char *>(&count), sizeof(std::size_t));
    Read(in, indexes_, count);
    Read(in, ordered_, count);
    Read(in, inserted_, count);
  }

  void save(std::string const &path) {
    std::ofstream out{path, std::ios::binary};
    std::size_t size = indexes_.size();
    out.write(reinterpret_cast<char *>(&size), sizeof(std::size_t));
    Write(out, indexes_);
    Write(out, ordered_);
    Write(out, inserted_);
  }

  std::size_t get_count() const { return indexes_.size(); }
  std::vector<std::size_t> const &get_indexes() const { return indexes_; }
  std::vector<T> const &get_inserted() const { return inserted_; }
  std::vector<T> const &get_ordered() const { return ordered_; }
};

template <typename T>
int generate_single(int argc, char **argv) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <count> <output path>\n";
    return EXIT_FAILURE;
  }

  std::size_t size = std::strtoul(argv[1], nullptr, 10);
  random_data_single<T> data{size};
  data.save(argv[2]);
  return EXIT_SUCCESS;
}
