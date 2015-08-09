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

class scoped_timer {
 private:
  using Clock = std::chrono::steady_clock;
  using Ns = std::chrono::nanoseconds;
  char const *description_;
  Clock::time_point start_;

 public:
  scoped_timer(char const *description)
      : description_{description}, start_{Clock::now()} {}
  ~scoped_timer() {
    auto stop = Clock::now();
    auto ns = std::chrono::duration_cast<Ns>(stop - start_);
    auto ms = ns.count() / 1000000.0;
    std::cout << std::setw(15) << std::setfill(' ') << std::setprecision(6)
              << ms << "ms " << description_ << "\n";
  }
};

template <typename F, typename... Args>
auto bench(char const *description, F functor, Args &&... args) ->
    typename std::result_of<F && (Args && ...)>::type {
  scoped_timer timer{description};
  return functor(std::forward<Args>(args)...);
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

template <typename Container, typename T>
void bench_iterator(Container const &container, std::vector<T> const &data) {
  verify(std::equal(container.begin(), container.end(), data.begin()), true);

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
