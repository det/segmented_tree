#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

template <typename F>
class scope_guard {
 private:
  bool active_;
  F f_;

 public:
  scope_guard(F &&f) : active_{true}, f_(std::forward<F>(f)) {}

  scope_guard(scope_guard &&other)
      : active_{other.active_}, f_{std::move(other.f_)} {
    other.dismiss();
  }

  ~scope_guard() noexcept {
    if (active_) f_();
  }

  void dismiss() { active_ = false; }
};

template <typename F>
scope_guard<F> make_scope_guard(F &&f) {
  return {std::forward<F>(f)};
}

template <typename F>
auto bench(char const *description, F f) -> typename std::result_of<F()>::type {
  using Clock = std::chrono::steady_clock;
  using Ns = std::chrono::nanoseconds;
  auto start = Clock::now();
  auto guard = make_scope_guard([&] {
    auto stop = Clock::now();
    auto ns = std::chrono::duration_cast<Ns>(stop - start);
    auto ms = ns.count() / 1000000.0;
    std::cout << std::fixed << std::setw(15) << std::setfill(' ')
              << std::setprecision(6) << ms << "ms " << description << "\n";
  });
  return f();
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
