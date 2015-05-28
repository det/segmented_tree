#include "adler64.hpp"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <type_traits>
#include <utility>
#include <vector>

template <typename T>
struct has_position {
 private:
  template <typename U>
  static auto test(U *t) -> decltype(t->position(0), std::true_type{});
  template <typename>
  static std::false_type test(...);

 public:
  static bool constexpr value = decltype(test<T>(nullptr))::value;
};

template <typename T>
typename std::enable_if<has_position<T>::value, typename T::iterator>::type
position(T &object, typename T::size_type pos) {
  return object.position(pos);
}

template <typename T>
typename std::enable_if<!has_position<T>::value, typename T::iterator>::type
position(T &object, typename T::size_type pos) {
  return object.begin() + pos;
}

template <typename T>
struct has_reserve {
 private:
  template <typename U>
  static auto test(U *t) -> decltype(t->reserve(0), std::true_type{});
  template <typename>
  static std::false_type test(...);

 public:
  static bool constexpr value = decltype(test<T>(nullptr))::value;
};

template <typename T>
typename std::enable_if<has_reserve<T>::value, void>::type reserve(
    T &object, typename T::size_type n) {
  return object.reserve(n);
}

template <typename T>
typename std::enable_if<!has_reserve<T>::value, void>::type reserve(
    T &, typename T::size_type) {}

template <typename Functor, typename... Args>
void bench(char const *description, Functor functor, Args &&... args) {
  auto start = std::chrono::high_resolution_clock::now();
  functor(std::forward<Args>(args)...);
  auto stop = std::chrono::high_resolution_clock::now();
  auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
  double ms = ns.count() / 1000000.0;
  std::cout << std::setw(15) << std::setfill(' ') << std::setprecision(6) << ms
            << "ms " << description << "\n";
}

template <typename Functor, typename... Args>
auto bench_value(char const *description, Functor functor, Args &&... args)
    -> decltype(functor(std::forward<Args>(args)...)) {
  auto start = std::chrono::high_resolution_clock::now();
  auto ret = functor(std::forward<Args>(args)...);
  auto stop = std::chrono::high_resolution_clock::now();
  auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
  double ms = ns.count() / 1000000.0;
  std::cout << std::setw(15) << std::setfill(' ') << std::setprecision(6) << ms
            << "ms " << description << "\n";
  return ret;
}

template <typename T>
void verify(T got, T expected) {
  if (got != expected) {
    std::string str;
    str += "expected: ";
    str += std::to_string(got);
    str += ", calculated: ";
    str += std::to_string(expected);
    throw std::runtime_error{str};
  }
}

template <typename T>
std::uint64_t accumulate_forward(T const &container) {
  adler64 adler;
  auto first = container.begin();
  auto last = container.end();

  while (first != last) {
    adler.update(*first);
    ++first;
  }

  return adler.final();
}

template <typename T>
std::uint64_t accumulate_forward_by(T const &container, std::size_t distance) {
  adler64 adler;
  auto left = container.size();
  auto first = container.begin();

  while (left >= distance) {
    adler.update(*first);
    first += distance;
    left -= distance;
  }

  return adler.final();
}

template <typename T>
std::uint64_t accumulate_backward(T const &container) {
  adler64 adler;
  auto first = container.begin();
  auto last = container.end();

  while (first != last) {
    --last;
    adler.update(*last);
  }
  return adler.final();
}

template <typename T>
std::uint64_t accumulate_backward_by(T const &container, std::size_t distance) {
  adler64 adler;
  auto left = container.size();
  auto it = container.end();

  while (left >= distance) {
    it -= distance;
    left -= distance;
    adler.update(*it);
  }

  return adler.final();
}

template <typename Container, typename T>
void bench_iterator(Container const &container, std::vector<T> const &data) {
  verify(accumulate_forward(data),
         bench_value("accumulate forward", accumulate_forward<Container>,
                     container));

  verify(accumulate_forward_by(data, 1),
         bench_value("accumulate forward by 1",
                     accumulate_forward_by<Container>, container, 1));

  verify(accumulate_forward_by(data, 10),
         bench_value("accumulate forward by 10",
                     accumulate_forward_by<Container>, container, 10));

  verify(accumulate_forward_by(data, 100),
         bench_value("accumulate forward by 100",
                     accumulate_forward_by<Container>, container, 100));

  verify(accumulate_forward_by(data, 1000),
         bench_value("accumulate forward by 1000",
                     accumulate_forward_by<Container>, container, 1000));

  verify(accumulate_forward_by(data, 10000),
         bench_value("accumulate forward by 10000",
                     accumulate_forward_by<Container>, container, 10000));

  verify(accumulate_backward(data),
         bench_value("accumulate backward", accumulate_backward<Container>,
                     container));

  verify(accumulate_backward_by(data, 1),
         bench_value("accumulate backward by 1",
                     accumulate_backward_by<Container>, container, 1));

  verify(accumulate_backward_by(data, 10),
         bench_value("accumulate backward by 10",
                     accumulate_backward_by<Container>, container, 10));

  verify(accumulate_backward_by(data, 100),
         bench_value("accumulate backward by 100",
                     accumulate_backward_by<Container>, container, 100));

  verify(accumulate_backward_by(data, 1000),
         bench_value("accumulate backward by 1000",
                     accumulate_backward_by<Container>, container, 1000));

  verify(accumulate_backward_by(data, 10000),
         bench_value("accumulate backward by 10000",
                     accumulate_backward_by<Container>, container, 10000));
}
