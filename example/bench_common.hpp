#include "adler64.hpp"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

template <typename T>
struct has_nth {
 private:
  template <typename U>
  static auto test(U *t) -> decltype(t->nth(0), std::true_type{});
  template <typename>
  static std::false_type test(...);

 public:
  static bool constexpr value = decltype(test<T>(nullptr))::value;
};

template <typename T>
typename std::enable_if<has_nth<T>::value, typename T::iterator>::type nth(
    T &object, typename T::size_type pos) {
  return object.nth(pos);
}

template <typename T>
typename std::enable_if<!has_nth<T>::value, typename T::iterator>::type nth(
    T &object, typename T::size_type pos) {
  return object.begin() + static_cast<std::ptrdiff_t>(pos);
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

template <typename T>
struct voidable {
  T value;
  template <typename F, typename... Args>
  voidable(F &&f, Args &&... args)
      : value{f(std::forward<Args>(args)...)} {}
  T &get() { return value; }
  T const &get() const { return value; }
};

template <>
struct voidable<void> {
  template <typename F, typename... Args>
  voidable(F &&f, Args &&... args) {
    f(std::forward<Args>(args)...);
  }
  void get() const {}
};

template <typename F, typename... Args>
auto make_voidable(F &&f, Args &&... args)
    -> voidable<typename std::result_of<F && (Args && ...)>::type> {
  return {std::forward<F>(f), std::forward<Args>(args)...};
}

template <typename F, typename... Args>
auto bench(char const *description, F functor, Args &&... args) ->
    typename std::result_of<F && (Args && ...)>::type {
  auto start = std::chrono::high_resolution_clock::now();
  auto ret = make_voidable(functor, std::forward<Args>(args)...);
  auto stop = std::chrono::high_resolution_clock::now();
  auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
  double ms = ns.count() / 1000000.0;
  std::cout << std::setw(15) << std::setfill(' ') << std::setprecision(6) << ms
            << "ms " << description << "\n";
  return ret.get();
}

template <typename T>
void verify(T got, T expected) {
  if (got != expected) {
    std::ostringstream stream;
    stream << "expected: " << got << ", calculated: " << expected;
    throw std::runtime_error{stream.str()};
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
    first += static_cast<std::ptrdiff_t>(distance);
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
    it -= static_cast<std::ptrdiff_t>(distance);
    left -= distance;
    adler.update(*it);
  }

  return adler.final();
}

template <typename Container, typename T>
void bench_iterator(Container const &container, std::vector<T> const &data) {
  verify(accumulate_forward(data), bench("accumulate forward", [&] {
    return accumulate_forward(container);
  }));

  verify(accumulate_forward_by(data, 1), bench("accumulate forward by 1", [&] {
    return accumulate_forward_by(container, 1);
  }));

  verify(accumulate_forward_by(data, 10),
         bench("accumulate forward by 10",
               [&] { return accumulate_forward_by(container, 10); }));

  verify(accumulate_forward_by(data, 100),
         bench("accumulate forward by 100",
               [&] { return accumulate_forward_by(container, 100); }));

  verify(accumulate_forward_by(data, 1000),
         bench("accumulate forward by 1000",
               [&] { return accumulate_forward_by(container, 1000); }));

  verify(accumulate_forward_by(data, 10000),
         bench("accumulate forward by 10000",
               [&] { return accumulate_forward_by(container, 10000); }));

  verify(accumulate_backward(data), bench("accumulate backward", [&] {
    return accumulate_backward(container);
  }));

  verify(accumulate_backward_by(data, 1),
         bench("accumulate backward by 1",
               [&] { return accumulate_backward_by(container, 1); }));

  verify(accumulate_backward_by(data, 10),
         bench("accumulate backward by 10",
               [&] { return accumulate_backward_by(container, 10); }));

  verify(accumulate_backward_by(data, 100),
         bench("accumulate backward by 100",
               [&] { return accumulate_backward_by(container, 100); }));

  verify(accumulate_backward_by(data, 1000),
         bench("accumulate backward by 1000",
               [&] { return accumulate_backward_by(container, 1000); }));

  verify(accumulate_backward_by(data, 10000),
         bench("accumulate backward by 10000",
               [&] { return accumulate_backward_by(container, 10000); }));
}
