#pragma once

#include <cstdint>

class adler64 {
 private:
  static constexpr std::uint64_t prime = 4294967291ULL;
  std::uint64_t a_ = 1;
  std::uint64_t b_ = 0;

 public:
  void update(std::uint64_t num) {
    a_ = (a_ + num) % prime;
    b_ = (b_ + a_) % prime;
  }

  std::uint64_t final() { return (b_ << 32) | a_; }
};
