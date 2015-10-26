#ifndef COMMON_ITERATOR
#define COMMON_ITERATOR

#include <cstdint>

template <typename T>
std::uint64_t accumulate_forward(T const &container) {
  std::uint64_t accu = 0;
  auto first = container.begin();
  auto last = container.end();

  while (first != last) {
    accu += *first;
    ++first;
  }

  return accu;
}

template <typename T>
std::uint64_t accumulate_forward_by(T const &container, std::size_t distance) {
  std::uint64_t accu = 0;
  auto left = container.size();
  auto first = container.begin();

  while (left >= distance) {
    accu += *first;
    first += static_cast<std::ptrdiff_t>(distance);
    left -= distance;
  }

  return accu;
}

template <typename T>
std::uint64_t accumulate_backward(T const &container) {
  std::uint64_t accu = 0;
  auto first = container.begin();
  auto last = container.end();

  while (first != last) {
    --last;
    accu += *last;
  }
  return accu;
}

template <typename T>
std::uint64_t accumulate_backward_by(T const &container, std::size_t distance) {
  std::uint64_t accu = 0;
  auto left = container.size();
  auto it = container.end();

  while (left >= distance) {
    it -= static_cast<std::ptrdiff_t>(distance);
    left -= distance;
    accu += *it;
  }

  return accu;
}

#endif  // #ifndef COMMON_ITERATOR
