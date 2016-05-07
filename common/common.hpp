#ifndef COMMON_COMMON
#define COMMON_COMMON

#include <cstdint>
#include <cstdlib>
#include <type_traits>
#include <vector>
#include <boost/crc.hpp>
#include <boost/random/independent_bits.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/seed_seq.hpp>
#include <random>

template <typename T>
struct insertion_data {
  std::vector<std::size_t> indexes;
  std::vector<T> ordered;
};

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

template <typename Rng, typename Result = typename Rng::result_type>
Result bounded_rand(Rng &rng, Result upper_bound) {
  Result threshold =
      (Rng::max() - Rng::min() + Result(1) - upper_bound) % upper_bound;
  while (true) {
    Result r = rng() - Rng::min();
    if (r >= threshold) return r % upper_bound;
  }
}

template <typename Container>
std::uint64_t make_checksum_unsigned(Container const &container) {
  boost::crc_optimal<64, 0x42F0E1EBA9EA3693ULL> accu;
  accu.process_block(container.data(), container.data() + container.size());
  return accu.checksum();
}

using seed_seq = boost::random::seed_seq;

using random_engine = boost::random::mt19937;

template <typename T>
using random_bits_engine =
    boost::random::independent_bits_engine<random_engine,
                                           std::numeric_limits<T>::digits, T>;

#endif  // #ifndef COMMON_COMMON
