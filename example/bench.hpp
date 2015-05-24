#include "adler64.hpp"
#include "random_data.hpp"

#include <cassert>
#include <chrono>
#include <iostream>
#include <iomanip>

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
void bench(std::string const &description, Functor functor, Args &&... args) {
  auto start = std::chrono::high_resolution_clock::now();
  functor(std::forward<Args>(args)...);
  auto stop = std::chrono::high_resolution_clock::now();
  auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
  double ms = ns.count() / 1000000.0;
  std::cout << std::setw(15) << std::setfill(' ') << std::setprecision(6) << ms
            << "ms " << description << "\n";
}

template <typename Functor, typename... Args>
auto bench_value(std::string const &description, Functor functor,
                 Args &&... args)
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

template <typename Container, typename T>
void insert_values(Container &container,
                   std::vector<std::size_t> const &indexes,
                   std::vector<T> const &data) {
  auto size = indexes.size();
  reserve(container, size);
  for (std::size_t i = 0; i != size; ++i) {
    container.insert(position(container, indexes[i]), data[i]);
  }
}

template <typename Container>
void erase_values(Container &container,
                  std::vector<std::size_t> const &indexes) {
  auto size = indexes.size();
  reserve(container, size);
  while (size != 1) {
    --size;
    container.erase(position(container, indexes[size]));
  }
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
void bench_container(random_data_t<T> const &data) {
  std::cout << std::fixed;
  Container container;

  bench("insert values", insert_values<Container, T>, container,
        data.get_indexes(), data.get_ordered());

  verify(accumulate_forward(data.get_inserted()),
         bench_value("accumulate forward", accumulate_forward<Container>,
                     container));

  verify(accumulate_forward_by(data.get_inserted(), 1),
         bench_value("accumulate forward by 1",
                     accumulate_forward_by<Container>, container, 1));

  verify(accumulate_forward_by(data.get_inserted(), 10),
         bench_value("accumulate forward by 10",
                     accumulate_forward_by<Container>, container, 10));

  verify(accumulate_forward_by(data.get_inserted(), 100),
         bench_value("accumulate forward by 100",
                     accumulate_forward_by<Container>, container, 100));

  verify(accumulate_forward_by(data.get_inserted(), 1000),
         bench_value("accumulate forward by 1000",
                     accumulate_forward_by<Container>, container, 1000));

  verify(accumulate_forward_by(data.get_inserted(), 10000),
         bench_value("accumulate forward by 10000",
                     accumulate_forward_by<Container>, container, 10000));

  verify(accumulate_backward(data.get_inserted()),
         bench_value("accumulate backward", accumulate_backward<Container>,
                     container));

  verify(accumulate_backward_by(data.get_inserted(), 1),
         bench_value("accumulate backward by 1",
                     accumulate_backward_by<Container>, container, 1));

  verify(accumulate_backward_by(data.get_inserted(), 10),
         bench_value("accumulate backward by 10",
                     accumulate_backward_by<Container>, container, 10));

  verify(accumulate_backward_by(data.get_inserted(), 100),
         bench_value("accumulate backward by 100",
                     accumulate_backward_by<Container>, container, 100));

  verify(accumulate_backward_by(data.get_inserted(), 1000),
         bench_value("accumulate backward by 1000",
                     accumulate_backward_by<Container>, container, 1000));

  verify(accumulate_backward_by(data.get_inserted(), 10000),
         bench_value("accumulate backward by 10000",
                     accumulate_backward_by<Container>, container, 10000));

  bench("erase values", erase_values<Container>, container, data.get_indexes());
  verify(data.get_ordered()[0], container[0]);
}

template <typename Container, typename T>
void test_compiles(random_data_t<T> const &data) {
  auto &inserted = data.get_inserted();
  Container{};
  auto c1 = Container(1000);
  auto c2 = Container{inserted.begin(), inserted.end()};
  auto c3 = Container{c2};
  auto c4 = std::move(c3);
  auto c5 = Container{1, 2, 3, 4};
  c1 = c4;
  c1 = std::move(c5);
  c4.assign(1000, 0);
  c1.assign(c4.begin(), c4.end());
  c4.assign({1, 2, 3, 4});
  c4.get_allocator();
  c4.at(2);
  const_cast<Container const &>(c4).at(2);
  c4[2];
  const_cast<Container const &>(c4)[2];
  c4.front();
  const_cast<Container const &>(c4).front();
  c4.back();
  const_cast<Container const &>(c4).back();
  c4.begin();
  const_cast<Container const &>(c4).begin();
  c4.cbegin();
  c4.penultimate();
  const_cast<Container const &>(c4).penultimate();
  c4.cpenultimate();
  c4.end();
  const_cast<Container const &>(c4).end();
  c4.cend();
  c4.position(1000);
  const_cast<Container const &>(c4).position(1000);
  c4.cposition(1000);
  c4.rbegin();
  const_cast<Container const &>(c4).rbegin();
  c4.crbegin();
  c4.rend();
  const_cast<Container const &>(c4).rend();
  c4.crend();
  c4.rposition(1000);
  const_cast<Container const &>(c4).rposition(1000);
  c4.crposition(1000);
  c4.empty();
  c4.size();
  c4.max_size();
  c4.clear();
  c4.insert(c4.end(), 0);
  T x = 0;
  c4.insert(c4.end(), std::move(x));
  c4.insert(c4.end(), 100, 0);
  c4.insert(c4.end(), c1.begin(), c1.end());
  c4.insert(c4.end(), {1, 2, 3, 4});
  c4.emplace(c4.end(), 0);
  c4.erase(c4.begin());
  c4.erase(c4.begin(), c4.end());
  c4.push_back(0);
  T y = 0;
  c4.push_back(std::move(y));
  c4.emplace_back(0);
  c4.pop_back();
  c4.push_front(0);
  T z = 0;
  c4.push_front(std::move(z));
  c4.emplace_front(0);
  c4.pop_front();
  c4.resize(10000);
  c4.resize(0);
  c4.swap(c1);
}
