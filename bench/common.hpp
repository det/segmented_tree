#ifndef BENCH_COMMON
#define BENCH_COMMON

#include <chrono>
#include <iostream>
#include <sstream>
#include <utility>

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
    std::cout << description << "," << ms << "\n";
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

#endif  // #ifndef BENCH_COMMON
