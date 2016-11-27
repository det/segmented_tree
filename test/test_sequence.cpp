#define BOOST_TEST_MODULE test_sequence

#include <boost/segmented_tree/seq.hpp>
#include <boost/test/unit_test.hpp>
#include <exception>
#include <limits>
#include "../common/iterator.hpp"
#include "../common/range.hpp"
#include "../common/single.hpp"

template <typename T, typename Alloc = std::allocator<T>>
using seq = boost::segmented_tree::seq<T, Alloc, TARGET_SIZE>;
using uint64_t = std::uint64_t;

template <typename T>
class tagged_allocator {
 private:
  unsigned tag_;

 public:
  using value_type = T;
  tagged_allocator(unsigned tag = 0) : tag_{tag} {}
  template <class U>
  tagged_allocator(tagged_allocator<U> const& other) : tag_{other.tag()} {}

  T* allocate(std::size_t n) {
    if (n <= std::numeric_limits<std::size_t>::max() / sizeof(T)) {
      if (auto ptr = std::malloc(n * sizeof(T))) return static_cast<T*>(ptr);
    }
    throw std::bad_alloc();
  }

  void deallocate(T* ptr, std::size_t) { std::free(ptr); }

  unsigned tag() const { return tag_; }
};

template <typename T, typename U>
inline bool operator==(const tagged_allocator<T>&, const tagged_allocator<U>&) {
  return true;
}

template <typename T, typename U>
inline bool operator!=(const tagged_allocator<T>& a,
                       const tagged_allocator<U>& b) {
  return !(a == b);
}

template <typename A, typename B>
void check_contents(A const& a, std::initializer_list<B> b) {
  BOOST_CHECK(a.size() == b.size());
  BOOST_CHECK(std::equal(a.begin(), a.end(), b.begin()));
}

template <typename A>
void check_contents(A const& a) {
  BOOST_CHECK(a.size() == 0);
  BOOST_CHECK(a.begin() == a.end());
}

BOOST_AUTO_TEST_CASE(test_construct_alloc) {
  tagged_allocator<uint64_t> alloc{12345};
  seq<uint64_t, tagged_allocator<uint64_t>> c1{alloc};
  check_contents(c1);
  BOOST_CHECK(c1.get_allocator().tag() == 12345);
}

BOOST_AUTO_TEST_CASE(test_construct_default) {
  seq<uint64_t> c1;
  check_contents(c1);
}

BOOST_AUTO_TEST_CASE(test_construct_count) {
  seq<uint64_t> c1(10, 3);
  check_contents(c1, {3, 3, 3, 3, 3, 3, 3, 3, 3, 3});
}

BOOST_AUTO_TEST_CASE(test_construct_range) {
  std::initializer_list<uint64_t> ilist{0, 1, 2, 3, 4};
  seq<uint64_t> c1{ilist.begin(), ilist.end()};
  check_contents(c1, ilist);
}

BOOST_AUTO_TEST_CASE(test_construct_copy) {
  std::initializer_list<uint64_t> ilist{0, 1, 2, 3, 4};
  seq<uint64_t> c1{ilist};
  seq<uint64_t> c2{c1};
  check_contents(c2, ilist);
}

BOOST_AUTO_TEST_CASE(test_construct_copy_alloc) {
  std::initializer_list<uint64_t> ilist{0, 1, 2, 3, 4};
  seq<uint64_t, tagged_allocator<uint64_t>> c1{ilist};
  tagged_allocator<uint64_t> alloc{12345};
  seq<uint64_t, tagged_allocator<uint64_t>> c2{c1, alloc};
  check_contents(c2, ilist);
  BOOST_CHECK(c2.get_allocator().tag() == 12345);
}

BOOST_AUTO_TEST_CASE(test_construct_move) {
  std::initializer_list<uint64_t> ilist{0, 1, 2, 3, 4};
  seq<uint64_t> c1{ilist};
  seq<uint64_t> c2{std::move(c1)};
  check_contents(c1);
  check_contents(c2, ilist);
}

BOOST_AUTO_TEST_CASE(test_construct_move_alloc) {
  std::initializer_list<uint64_t> ilist{0, 1, 2, 3, 4};
  seq<uint64_t, tagged_allocator<uint64_t>> c1{ilist};
  tagged_allocator<uint64_t> alloc{12345};
  seq<uint64_t, tagged_allocator<uint64_t>> c2{std::move(c1), alloc};
  check_contents(c1);
  check_contents(c2, ilist);
  BOOST_CHECK(c2.get_allocator().tag() == 12345);
}

BOOST_AUTO_TEST_CASE(test_construct_ilist) {
  std::initializer_list<uint64_t> ilist{0, 1, 2, 3, 4};
  seq<uint64_t> c1{ilist};
  check_contents(c1, ilist);
}

BOOST_AUTO_TEST_CASE(test_operator_assign) {
  std::initializer_list<uint64_t> ilist1{0, 1, 2, 3, 4};
  std::initializer_list<uint64_t> ilist2{5, 4, 3};
  seq<uint64_t> c1{ilist1};
  seq<uint64_t> c2{ilist2};
  seq<uint64_t> c3;
  c3 = c1;
  check_contents(c3, ilist1);
  c3 = c2;
  check_contents(c3, ilist2);
}

BOOST_AUTO_TEST_CASE(test_operator_move_assign) {
  std::initializer_list<uint64_t> ilist1{0, 1, 2, 3, 4};
  std::initializer_list<uint64_t> ilist2{5, 4, 3};
  seq<uint64_t> c1{ilist1};
  seq<uint64_t> c2{ilist2};
  seq<uint64_t> c3;
  c3 = std::move(c1);
  check_contents(c3, ilist1);
  check_contents(c1);
  c3 = std::move(c2);
  check_contents(c3, ilist2);
  check_contents(c2);
}

BOOST_AUTO_TEST_CASE(test_operator_assign_ilist) {
  std::initializer_list<uint64_t> ilist1{0, 1, 2, 3, 4};
  std::initializer_list<uint64_t> ilist2{5, 4, 3};
  seq<uint64_t> c1;
  c1 = ilist1;
  check_contents(c1, ilist1);
  c1 = ilist2;
  check_contents(c1, ilist2);
}

BOOST_AUTO_TEST_CASE(test_assign_count) {
  seq<uint64_t> c1{0, 1, 2, 3, 4};
  c1.assign(10, 0);
  check_contents(c1, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0});
  c1.assign(5, 1);
  check_contents(c1, {1, 1, 1, 1, 1});
}

BOOST_AUTO_TEST_CASE(test_assign_range) {
  std::initializer_list<uint64_t> ilist{0, 1, 2, 3, 4};
  seq<uint64_t> c1;
  c1.assign(ilist.begin(), ilist.end());
  check_contents(c1, ilist);
}

BOOST_AUTO_TEST_CASE(test_assign_ilist) {
  std::initializer_list<uint64_t> ilist{0, 1, 2, 3, 4};
  seq<uint64_t> c1;
  c1.assign(ilist);
  check_contents(c1, ilist);
}

BOOST_AUTO_TEST_CASE(test_get_allocator) {
  tagged_allocator<uint64_t> alloc{12345};
  seq<uint64_t, tagged_allocator<uint64_t>> c1{alloc};
  BOOST_CHECK(c1.get_allocator().tag() == 12345);
}

template <typename T>
bool check_exception(T const&) {
  return true;
}

BOOST_AUTO_TEST_CASE(test_at) {
  seq<uint64_t> c1{0};
  BOOST_CHECK(c1.at(0) == 0);
  BOOST_CHECK_EXCEPTION(c1.at(1), std::out_of_range,
                        check_exception<std::out_of_range>);

  auto const& view = c1;
  BOOST_CHECK(view.at(0) == 0);
  BOOST_CHECK_EXCEPTION(view.at(1), std::out_of_range,
                        check_exception<std::out_of_range>);
}

BOOST_AUTO_TEST_CASE(test_operator_index) {
  seq<uint64_t> c1{0, 1};
  BOOST_CHECK(c1[0] == 0);
  BOOST_CHECK(c1[1] == 1);

  auto const& view = c1;
  BOOST_CHECK(view[0] == 0);
  BOOST_CHECK(view[1] == 1);
}

BOOST_AUTO_TEST_CASE(test_front) {
  seq<uint64_t> c1{0, 1};
  BOOST_CHECK(c1.front() == 0);

  auto const& view = c1;
  BOOST_CHECK(view.front() == 0);
}

BOOST_AUTO_TEST_CASE(test_back) {
  seq<uint64_t> c1{0, 1};
  BOOST_CHECK(c1.back() == 1);

  auto const& view = c1;
  BOOST_CHECK(view.back() == 1);
}

BOOST_AUTO_TEST_CASE(test_begin) {
  seq<uint64_t> c1{0, 1};
  BOOST_CHECK(*c1.begin() == 0);
  BOOST_CHECK(*c1.begin() == 0);

  auto const& view = c1;
  BOOST_CHECK(*view.begin() == 0);
  BOOST_CHECK(*view.begin() == 0);
}

BOOST_AUTO_TEST_CASE(test_end) {
  seq<uint64_t> c1{0, 1};
  BOOST_CHECK(*(c1.end() - 1) == 1);
  BOOST_CHECK(*(c1.end() - 1) == 1);

  auto const& view = c1;
  BOOST_CHECK(*(view.end() - 1) == 1);
  BOOST_CHECK(*(view.end() - 1) == 1);
}

BOOST_AUTO_TEST_CASE(test_rbegin) {
  seq<uint64_t> c1{0, 1};
  BOOST_CHECK(*c1.rbegin() == 1);
  BOOST_CHECK(*c1.rbegin() == 1);

  auto const& view = c1;
  BOOST_CHECK(*view.rbegin() == 1);
  BOOST_CHECK(*view.rbegin() == 1);
}

BOOST_AUTO_TEST_CASE(test_rend) {
  seq<uint64_t> c1{0, 1};
  BOOST_CHECK(*(c1.rend() - 1) == 0);
  BOOST_CHECK(*(c1.rend() - 1) == 0);

  auto const& view = c1;
  BOOST_CHECK(*(view.rend() - 1) == 0);
  BOOST_CHECK(*(view.rend() - 1) == 0);
}

BOOST_AUTO_TEST_CASE(test_penultimate) {
  seq<uint64_t> c1{0, 1};
  BOOST_CHECK(*c1.penultimate() == 1);
  BOOST_CHECK(*c1.cpenultimate() == 1);

  auto const& view = c1;
  BOOST_CHECK(*view.penultimate() == 1);
  BOOST_CHECK(*view.cpenultimate() == 1);
}

BOOST_AUTO_TEST_CASE(test_nth) {
  seq<uint64_t> c1{0, 1};
  BOOST_CHECK(*c1.nth(0) == 0);
  BOOST_CHECK(*c1.nth(1) == 1);

  auto const& view = c1;
  BOOST_CHECK(*view.nth(0) == 0);
  BOOST_CHECK(*view.nth(1) == 1);
}

BOOST_AUTO_TEST_CASE(test_index_of) {
  seq<uint64_t> c1{0, 1};
  BOOST_CHECK(c1.index_of(c1.nth(0)) == 0);
  BOOST_CHECK(c1.index_of(c1.nth(1)) == 1);
  BOOST_CHECK(c1.index_of(c1.nth(2)) == 2);

  auto const& view = c1;
  BOOST_CHECK(view.index_of(c1.nth(0)) == 0);
  BOOST_CHECK(view.index_of(c1.nth(1)) == 1);
  BOOST_CHECK(view.index_of(c1.nth(2)) == 2);
}

BOOST_AUTO_TEST_CASE(test_empty) {
  seq<uint64_t> c1;
  BOOST_CHECK(c1.empty() == true);
  seq<uint64_t> c2{0};
  BOOST_CHECK(c2.empty() == false);
}

BOOST_AUTO_TEST_CASE(test_size) {
  seq<uint64_t> c1;
  BOOST_CHECK(c1.size() == 0);
  seq<uint64_t> c2{0};
  BOOST_CHECK(c2.size() == 1);
  seq<uint64_t> c3{0, 1, 2, 3, 4};
  BOOST_CHECK(c3.size() == 5);
}

BOOST_AUTO_TEST_CASE(test_height) {
  seq<uint64_t> c1;
  BOOST_CHECK(c1.height() == 0);
  seq<uint64_t> c2{0};
  BOOST_CHECK(c2.height() == 1);
}

BOOST_AUTO_TEST_CASE(test_max_size) {
  seq<uint64_t> c1;
  BOOST_CHECK(c1.max_size() == std::numeric_limits<std::size_t>::max());
}

BOOST_AUTO_TEST_CASE(test_clear) {
  seq<uint64_t> c1{0, 1, 2, 3, 4};
  c1.clear();
  check_contents(c1);
}

BOOST_AUTO_TEST_CASE(test_insert_lvalue) {
  seq<uint64_t> c1{0, 1, 2, 4};
  c1.insert(c1.nth(3), 3);
  check_contents(c1, {0, 1, 2, 3, 4});

  seq<std::string> c2{"zero", "one", "two", "four"};
  c2.insert(c2.nth(3), "three");
  check_contents(c2, {"zero", "one", "two", "three", "four"});
}

BOOST_AUTO_TEST_CASE(test_insert_rvalue) {
  seq<std::string> c1{"zero", "one", "two", "four"};
  std::string s = "three";
  c1.insert(c1.nth(3), std::move(s));
  check_contents(c1, {"zero", "one", "two", "three", "four"});
}

BOOST_AUTO_TEST_CASE(test_insert_lvalue_count) {
  seq<uint64_t> c1{0, 1, 2, 4};
  c1.insert(c1.nth(3), 3, 3);
  check_contents(c1, {0, 1, 2, 3, 3, 3, 4});
}

BOOST_AUTO_TEST_CASE(test_insert_range) {
  seq<uint64_t> c1{0, 1, 2, 6};
  std::initializer_list<uint64_t> ilist{3, 4, 5};
  c1.insert(c1.nth(3), ilist.begin(), ilist.end());
  check_contents(c1, {0, 1, 2, 3, 4, 5, 6});
}

BOOST_AUTO_TEST_CASE(test_insert_ilist) {
  seq<uint64_t> c1{0, 1, 2, 6};
  std::initializer_list<uint64_t> ilist{3, 4, 5};
  c1.insert(c1.nth(3), ilist);
  check_contents(c1, {0, 1, 2, 3, 4, 5, 6});
}

BOOST_AUTO_TEST_CASE(test_emplace) {
  seq<uint64_t> c1{0, 1, 2, 4};
  c1.emplace(c1.nth(3));
  check_contents(c1, {0, 1, 2, 0, 4});

  seq<std::string> c2{"zero", "one", "two", "four"};
  c2.emplace(c2.nth(3), "three");
  check_contents(c2, {"zero", "one", "two", "three", "four"});
}

BOOST_AUTO_TEST_CASE(test_erase) {
  seq<uint64_t> c1{0, 1, 2, 3, 4};
  c1.erase(c1.nth(3));
  check_contents(c1, {0, 1, 2, 4});
  c1.erase(c1.nth(1));
  check_contents(c1, {0, 2, 4});
  c1.erase(c1.nth(2));
  check_contents(c1, {0, 2});
  c1.erase(c1.nth(0));
  check_contents(c1, {2});
  c1.erase(c1.nth(0));
  check_contents(c1);

  seq<std::string> c2{"zero", "one", "two", "three", "four"};
  c2.erase(c2.nth(3));
  check_contents(c2, {"zero", "one", "two", "four"});
  c2.erase(c2.nth(1));
  check_contents(c2, {"zero", "two", "four"});
  c2.erase(c2.nth(2));
  check_contents(c2, {"zero", "two"});
  c2.erase(c2.nth(0));
  check_contents(c2, {"two"});
  c2.erase(c2.nth(0));
  check_contents(c2);
}

BOOST_AUTO_TEST_CASE(test_erase_range) {
  seq<uint64_t> c1{0, 1, 2, 3, 4};
  c1.erase(c1.nth(1), c1.nth(4));
  check_contents(c1, {0, 4});
}

BOOST_AUTO_TEST_CASE(test_push_back_lvalue) {
  seq<uint64_t> c1;
  c1.push_back(0);
  c1.push_back(1);
  check_contents(c1, {0, 1});
}

BOOST_AUTO_TEST_CASE(test_push_back_rvalue) {
  seq<uint64_t> c1;
  uint64_t a1 = 0;
  uint64_t b1 = 1;
  c1.push_back(std::move(a1));
  c1.push_back(std::move(b1));
  check_contents(c1, {0, 1});

  seq<std::string> c2;
  std::string a2 = "zero";
  std::string b2 = "one";
  c2.push_back(std::move(a2));
  c2.push_back(std::move(b2));
  check_contents(c2, {"zero", "one"});
}

BOOST_AUTO_TEST_CASE(test_emplace_back) {
  seq<uint64_t> c1;
  c1.emplace_back();
  c1.emplace_back(1);
  check_contents(c1, {0, 1});

  seq<std::string> c2;
  c2.emplace_back();
  c2.emplace_back("one");
  check_contents(c2, {"", "one"});
}

BOOST_AUTO_TEST_CASE(test_pop_back) {
  seq<uint64_t> c1{0, 1, 2, 3, 4};
  c1.pop_back();
  c1.pop_back();
  check_contents(c1, {0, 1, 2});
}

BOOST_AUTO_TEST_CASE(test_push_front_lvalue) {
  seq<uint64_t> c1;
  c1.push_front(1);
  c1.push_front(0);
  check_contents(c1, {0, 1});
}

BOOST_AUTO_TEST_CASE(test_push_front_rvalue) {
  seq<uint64_t> c1;
  uint64_t b1 = 1;
  uint64_t a1 = 0;
  c1.push_front(std::move(b1));
  c1.push_front(std::move(a1));
  check_contents(c1, {0, 1});

  seq<std::string> c2;
  std::string b2 = "one";
  std::string a2 = "zero";
  c2.push_front(std::move(b2));
  c2.push_front(std::move(a2));
  check_contents(c2, {"zero", "one"});
}

BOOST_AUTO_TEST_CASE(test_emplace_front) {
  seq<uint64_t> c1;
  c1.emplace_front(1);
  c1.emplace_front();
  check_contents(c1, {0, 1});

  seq<std::string> c2;
  c2.emplace_front("one");
  c2.emplace_front();
  check_contents(c2, {"", "one"});
}

BOOST_AUTO_TEST_CASE(test_pop_front) {
  seq<uint64_t> c1{0, 1, 2, 3, 4};
  c1.pop_front();
  c1.pop_front();
  check_contents(c1, {2, 3, 4});
}

BOOST_AUTO_TEST_CASE(test_resize_default) {
  seq<uint64_t> c1{0, 1, 2, 3, 4};
  c1.resize(10);
  check_contents(c1, {0, 1, 2, 3, 4, 0, 0, 0, 0, 0});
  c1.resize(5);
  check_contents(c1, {0, 1, 2, 3, 4});
}

BOOST_AUTO_TEST_CASE(test_resize_value) {
  seq<uint64_t> c1{0, 1, 2, 3, 4};
  c1.resize(10, 1);
  check_contents(c1, {0, 1, 2, 3, 4, 1, 1, 1, 1, 1});
  c1.resize(5, 1);
  check_contents(c1, {0, 1, 2, 3, 4});
}

BOOST_AUTO_TEST_CASE(test_swap_member) {
  std::initializer_list<uint64_t> a{0, 1, 2, 3, 4};
  std::initializer_list<uint64_t> b{4, 3, 2, 1, 0};
  seq<uint64_t> c1{a};
  seq<uint64_t> c2{b};
  c1.swap(c2);
  check_contents(c1, b);
  check_contents(c2, a);
}

BOOST_AUTO_TEST_CASE(test_swap_free) {
  std::initializer_list<uint64_t> a{0, 1, 2, 3, 4};
  std::initializer_list<uint64_t> b{4, 3, 2, 1, 0};
  seq<uint64_t> c1{a};
  seq<uint64_t> c2{b};
  swap(c1, c2);
  check_contents(c1, b);
  check_contents(c2, a);
}

BOOST_AUTO_TEST_CASE(test_splice_lvalue) {
  seq<uint64_t> c1{0, 1, 2, 3, 4};
  seq<uint64_t> c2{5, 6, 7, 8, 9};
  c1.splice(c1.nth(3), c2);
  check_contents(c1, {0, 1, 2, 5, 6, 7, 8, 9, 3, 4});
  check_contents(c2);

  seq<std::string> c3{"zero", "one", "two", "three", "four"};
  seq<std::string> c4{"five", "six", "seven", "eight", "nine"};
  c3.splice(c3.nth(3), c4);
  check_contents(c3, {"zero", "one", "two", "five", "six", "seven", "eight",
                      "nine", "three", "four"});
  check_contents(c4);
}

BOOST_AUTO_TEST_CASE(test_splice_rvalue) {
  seq<uint64_t> c1{0, 1, 2, 3, 4};
  seq<uint64_t> c2{5, 6, 7, 8, 9};
  c1.splice(c1.nth(3), std::move(c2));
  check_contents(c1, {0, 1, 2, 5, 6, 7, 8, 9, 3, 4});
  check_contents(c2);

  seq<std::string> c3{"zero", "one", "two", "three", "four"};
  seq<std::string> c4{"five", "six", "seven", "eight", "nine"};
  c3.splice(c3.nth(3), std::move(c4));
  check_contents(c3, {"zero", "one", "two", "five", "six", "seven", "eight",
                      "nine", "three", "four"});
  check_contents(c4);
}

BOOST_AUTO_TEST_CASE(test_splice_single_lvalue) {
  seq<uint64_t> c1{0, 1, 2, 3, 4};
  seq<uint64_t> c2{5, 6, 7, 8, 9};
  c1.splice(c1.nth(3), c2, c2.nth(3));
  check_contents(c1, {0, 1, 2, 8, 3, 4});
  check_contents(c2, {5, 6, 7, 9});

  seq<std::string> c3{"zero", "one", "two", "three", "four"};
  seq<std::string> c4{"five", "six", "seven", "eight", "nine"};
  c3.splice(c3.nth(3), c4, c4.nth(3));
  check_contents(c3, {"zero", "one", "two", "eight", "three", "four"});
  check_contents(c4, {"five", "six", "seven", "nine"});
}

BOOST_AUTO_TEST_CASE(test_splice_single_rvalue) {
  seq<uint64_t> c1{0, 1, 2, 3, 4};
  seq<uint64_t> c2{5, 6, 7, 8, 9};
  c1.splice(c1.nth(3), std::move(c2), c2.nth(3));
  check_contents(c1, {0, 1, 2, 8, 3, 4});
  check_contents(c2, {5, 6, 7, 9});

  seq<std::string> c3{"zero", "one", "two", "three", "four"};
  seq<std::string> c4{"five", "six", "seven", "eight", "nine"};
  c3.splice(c3.nth(3), std::move(c4), c4.nth(3));
  check_contents(c3, {"zero", "one", "two", "eight", "three", "four"});
  check_contents(c4, {"five", "six", "seven", "nine"});
}

BOOST_AUTO_TEST_CASE(test_splice_range_lvalue) {
  seq<uint64_t> c1{0, 1, 2, 3, 4};
  seq<uint64_t> c2{5, 6, 7, 8, 9};
  c1.splice(c1.nth(3), c2, c2.nth(1), c2.nth(4));
  check_contents(c1, {0, 1, 2, 6, 7, 8, 3, 4});
  check_contents(c2, {5, 9});

  seq<std::string> c3{"zero", "one", "two", "three", "four"};
  seq<std::string> c4{"five", "six", "seven", "eight", "nine"};
  c3.splice(c3.nth(3), c4, c4.nth(1), c4.nth(4));
  check_contents(
      c3, {"zero", "one", "two", "six", "seven", "eight", "three", "four"});
  check_contents(c4, {"five", "nine"});
}

BOOST_AUTO_TEST_CASE(test_splice_range_rvalue) {
  seq<uint64_t> c1{0, 1, 2, 3, 4};
  seq<uint64_t> c2{5, 6, 7, 8, 9};
  c1.splice(c1.nth(3), std::move(c2), c2.nth(1), c2.nth(4));
  check_contents(c1, {0, 1, 2, 6, 7, 8, 3, 4});
  check_contents(c2, {5, 9});

  seq<std::string> c3{"zero", "one", "two", "three", "four"};
  seq<std::string> c4{"five", "six", "seven", "eight", "nine"};
  c3.splice(c3.nth(3), std::move(c4), c4.nth(1), c4.nth(4));
  check_contents(
      c3, {"zero", "one", "two", "six", "seven", "eight", "three", "four"});
  check_contents(c4, {"five", "nine"});
}

BOOST_AUTO_TEST_CASE(test_splice_merge_lvalue) {
  seq<uint64_t> c1{0, 1, 2, 10, 20, 21, 30};
  seq<uint64_t> c2{1, 3, 4, 5, 22, 29, 31};
  c1.merge(c2);
  check_contents(c1, {0, 1, 1, 2, 3, 4, 5, 10, 20, 21, 22, 29, 30, 31});
  check_contents(c2);
}

BOOST_AUTO_TEST_CASE(test_splice_merge_rvalue) {
  seq<uint64_t> c1{0, 1, 2, 10, 20, 21, 30};
  seq<uint64_t> c2{1, 3, 4, 5, 22, 29, 31};
  c1.merge(std::move(c2));
  check_contents(c1, {0, 1, 1, 2, 3, 4, 5, 10, 20, 21, 22, 29, 30, 31});
  check_contents(c2);
}

BOOST_AUTO_TEST_CASE(test_remove) {
  seq<uint64_t> c1{0, 1, 2, 3, 4, 4, 3, 2, 1, 0};
  c1.remove(2);
  check_contents(c1, {0, 1, 3, 4, 4, 3, 1, 0});
}

BOOST_AUTO_TEST_CASE(test_remove_if) {
  seq<uint64_t> c1{0, 1, 2, 3, 4, 4, 3, 2, 1, 0};
  c1.remove_if([](uint64_t data) { return data >= 2; });
  check_contents(c1, {0, 1, 1, 0});
}

BOOST_AUTO_TEST_CASE(test_reverse) {
  seq<uint64_t> c1{0, 1, 2, 3, 4};
  c1.reverse();
  check_contents(c1, {4, 3, 2, 1, 0});
}

BOOST_AUTO_TEST_CASE(test_unique) {
  seq<uint64_t> c1{0, 1, 1, 2, 2, 2, 3, 4};
  c1.unique();
  check_contents(c1, {0, 1, 2, 3, 4});
}

BOOST_AUTO_TEST_CASE(test_sort) {
  seq<uint64_t> c1{3, 0, 4, 1, 2};
  c1.sort();
  check_contents(c1, {0, 1, 2, 3, 4});
}

BOOST_AUTO_TEST_CASE(test_sort_predicate) {
  seq<uint64_t> c1{3, 0, 4, 1, 2};
  c1.sort([](uint64_t a, uint64_t b) { return a > b; });
  check_contents(c1, {4, 3, 2, 1, 0});
}

template <typename Container, typename T>
void test_iterator(Container const& container, std::vector<T> const& data) {
  BOOST_CHECK(accumulate_forward(data) == accumulate_forward(container));

  BOOST_CHECK(accumulate_forward_by(data, 1) ==
              accumulate_forward_by(container, 1));

  BOOST_CHECK(accumulate_forward_by(data, 10) ==
              accumulate_forward_by(container, 10));

  BOOST_CHECK(accumulate_forward_by(data, 100) ==
              accumulate_forward_by(container, 100));

  BOOST_CHECK(accumulate_forward_by(data, 1000) ==
              accumulate_forward_by(container, 1000));

  BOOST_CHECK(accumulate_forward_by(data, 10000) ==
              accumulate_forward_by(container, 10000));

  BOOST_CHECK(accumulate_backward(data) == accumulate_backward(container));

  BOOST_CHECK(accumulate_backward_by(data, 1) ==
              accumulate_backward_by(container, 1));

  BOOST_CHECK(accumulate_backward_by(data, 10) ==
              accumulate_backward_by(container, 10));

  BOOST_CHECK(accumulate_backward_by(data, 100) ==
              accumulate_backward_by(container, 100));

  BOOST_CHECK(accumulate_backward_by(data, 1000) ==
              accumulate_backward_by(container, 1000));

  BOOST_CHECK(accumulate_backward_by(data, 10000) ==
              accumulate_backward_by(container, 10000));
}

template <typename T>
void test_single(std::size_t count, std::uint32_t seed,
                 std::uint64_t checksum) {
  auto data = make_insertion_data_single<T>(count, seed);
  seq<T> container;
  insert_single(container, data);
  std::vector<T> inserted{container.begin(), container.end()};
  BOOST_CHECK(checksum == make_checksum_unsigned(inserted));
  test_iterator(container, inserted);
  erase_single(container, data);
  BOOST_CHECK(container.size() == std::size_t{1});
  BOOST_CHECK(container[0] == data.ordered[0]);
}

BOOST_AUTO_TEST_CASE(test_random_single) {
  test_single<uint64_t>(32ULL, 2397254571ULL, 4723602420748635361ULL);
  test_single<uint64_t>(992ULL, 463092544ULL, 12966777589746855639ULL);
  test_single<uint64_t>(30752ULL, 430452927ULL, 751509891372566603ULL);
  test_single<uint64_t>(953312ULL, 3109453262ULL, 10176667110359292238ULL);
}

template <typename T>
void test_range(std::size_t count, std::size_t size, std::uint32_t seed,
                std::uint64_t checksum) {
  auto data = make_insertion_data_range<T>(count, size, seed);
  seq<T> container;
  insert_range(container, data);
  std::vector<T> inserted{container.begin(), container.end()};
  BOOST_CHECK(checksum == make_checksum_unsigned(inserted));
  test_iterator(container, inserted);
  erase_range(container, data);
  BOOST_CHECK(std::size_t{1} == container.size());
  BOOST_CHECK(data.ordered[0] == container[0]);
}

BOOST_AUTO_TEST_CASE(test_random_range) {
  test_range<uint64_t>(1ULL, 953312ULL, 235951511ULL, 7803621008785366632ULL);
  test_range<uint64_t>(31ULL, 30752ULL, 1082972474ULL, 11846815057285548515ULL);
  test_range<uint64_t>(961ULL, 992ULL, 5659033ULL, 14482810490810820797ULL);
  test_range<uint64_t>(29791ULL, 32ULL, 3727649439ULL, 10804193997107502541ULL);
}

struct retry_exception {};

template <typename T>
class throwing_allocator {
 private:
  random_engine engine_;

 public:
  using value_type = T;

  throwing_allocator() = default;
  template <class U>
  throwing_allocator(const throwing_allocator<U>&) {}

  T* allocate(std::size_t n) {
    if (engine_() % 8 == 0) throw retry_exception{};
    if (n <= std::numeric_limits<std::size_t>::max() / sizeof(T)) {
      if (auto ptr = std::malloc(n * sizeof(T))) return static_cast<T*>(ptr);
    }
    throw std::bad_alloc();
  }

  void deallocate(T* ptr, std::size_t) { std::free(ptr); }

  template <typename U, typename... Args>
  void construct(U* p, Args&&... args) {
    if (engine_() % 8 == 0) throw retry_exception{};
    new (p) U(std::forward<Args>(args)...);
  }

  template <typename U, typename... Args>
  void construct(U* p, T&& value) {
    new (p) U(std::move(value));
  }
};

template <typename T, typename U>
inline bool operator==(const throwing_allocator<T>&,
                       const throwing_allocator<U>&) {
  return true;
}

template <typename T, typename U>
inline bool operator!=(const throwing_allocator<T>& a,
                       const throwing_allocator<U>& b) {
  return !(a == b);
}

template <typename Container, typename T>
void insert_single_retry(Container& container, insertion_data<T> const& data) {
  auto count = data.indexes.size();
  reserve(container, count);
  for (std::size_t i = 0; i != count; ++i) {
    while (true) {
      try {
        container.insert(nth(container, data.indexes[i]), data.ordered[i]);
        break;
      } catch (retry_exception const&) {
      }
    }
  }
}

template <typename T>
void test_single_retry(std::size_t count, std::uint32_t seed,
                       std::uint64_t checksum) {
  auto data = make_insertion_data_single<T>(count, seed);
  seq<T, throwing_allocator<T>> container;
  insert_single_retry(container, data);
  std::vector<T> inserted{container.begin(), container.end()};
  BOOST_CHECK(checksum == make_checksum_unsigned(inserted));
  test_iterator(container, inserted);
  erase_single(container, data);
  BOOST_CHECK(container.size() == std::size_t{1});
  BOOST_CHECK(container[0] == data.ordered[0]);
}

BOOST_AUTO_TEST_CASE(test_random_single_retry) {
  test_single_retry<uint64_t>(32ULL, 2397254571ULL, 4723602420748635361ULL);
  test_single_retry<uint64_t>(992ULL, 463092544ULL, 12966777589746855639ULL);
  test_single_retry<uint64_t>(30752ULL, 430452927ULL, 751509891372566603ULL);
  test_single_retry<uint64_t>(953312ULL, 3109453262ULL,
                              10176667110359292238ULL);
}

BOOST_AUTO_TEST_CASE(test_traits) {
  auto x = boost::segmented_tree::detail::is_alloc_move_construct_default<
      int, throwing_allocator<int>>::value;
  auto y = boost::segmented_tree::detail::is_alloc_move_construct_default<
      int, tagged_allocator<int>>::value;
  BOOST_CHECK(x == false);
  BOOST_CHECK(y == true);
}
