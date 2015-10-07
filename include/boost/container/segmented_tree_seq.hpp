// (C) Copyright Chris Clearwater 2014-2015. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy
// at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_CONTAINER_SEGMENTED_TREE_SEQ
#define BOOST_CONTAINER_SEGMENTED_TREE_SEQ

#include "segmented_tree_seq_fwd.hpp"

#include <array>
#include <algorithm>
#include <cstring>
#include <limits>
#include <string>
#include <tuple>
#include <utility>
#include <vector>
#include <type_traits>

#ifdef BOOST_SEGMENTED_TREE_SEQ_DEBUG
#include <iostream>
#endif

namespace boost {
namespace container {
namespace segmented_tree_seq_detail {
#ifndef BOOST_CONTAINER_DOXYGEN_INVOKED
namespace is_nothrow_swappable_impl {
using std::swap;

template <typename T>
struct test {
  static bool const value =
      noexcept(swap(std::declval<T &>(), std::declval<T &>()));
};
}

template <typename T>
struct is_nothrow_swappable
    : std::integral_constant<bool, is_nothrow_swappable_impl::test<T>::value> {
};
#endif  // BOOST_CONTAINER_DOXYGEN_INVOKED

template <typename T, typename VoidPointer, typename SizeType,
          std::size_t segment_target, std::size_t base_target>
struct static_traits_t {
#ifndef BOOST_CONTAINER_DOXYGEN_INVOKED
  // forward declarations
  struct node_base;
  struct node_data;
  struct node;
  struct segment_entry;
  struct leaf_entry;
  struct iterator_entry;
  struct iterator_data;
  template <typename, typename>
  class iterator_t;

  using value_type = T;
  using void_pointer = VoidPointer;
  using size_type = SizeType;
  using node_pointer =
      typename std::pointer_traits<VoidPointer>::template rebind<node>;
  using element_pointer =
      typename std::pointer_traits<VoidPointer>::template rebind<value_type>;
  using difference_type =
      typename std::pointer_traits<VoidPointer>::difference_type;

  // sized types
  struct node_base {
    node_pointer parent_pointer;
    std::uint16_t parent_index;
    std::uint16_t length;
  };

  struct node_data {
    void_pointer pointer;
    size_type sz;
  };

  // constexpr
  static constexpr auto segment_free = segment_target;
  static constexpr auto node_size = sizeof(node_base);
  static constexpr auto base_free =
      node_size > base_target ? 0 : base_target - node_size;
  static constexpr auto segment_fit = segment_free / sizeof(T);
  static constexpr auto base_fit = base_free / sizeof(node_data);
  static constexpr auto segment_max = segment_fit > 1 ? segment_fit : 1;
  static constexpr auto base_max = base_fit > 3 ? base_fit : 3;
  static constexpr auto segment_min = (segment_max + 1) / 2;
  static constexpr auto base_min = (base_max + 1) / 2;

  // types
  struct node {
    node_pointer parent_pointer;
    std::uint16_t parent_index_;
    std::uint16_t length_;
    std::array<size_type, base_max> sizes;
    std::array<void_pointer, base_max> pointers;

    size_type parent_index() { return parent_index_; }

    void parent_index(size_type index) {
      parent_index_ = static_cast<std::uint16_t>(index);
    }

    size_type length() { return length_; }

    void length(size_type length) {
      length_ = static_cast<std::uint16_t>(length);
    }
  };

  struct segment_entry {
    element_pointer pointer;
    size_type index;
    size_type length;
  };

  struct leaf_entry {
    node_pointer pointer;
    size_type index;
  };

  struct iterator_entry {
    segment_entry segment;
    leaf_entry leaf;
  };

  struct iterator_data {
    iterator_entry entry;
    size_type pos;
  };

  // cast
  static element_pointer cast_segment(void_pointer pointer) {
    return static_cast<element_pointer>(pointer);
  }

  static node_pointer cast_node(void_pointer pointer) {
    return static_cast<node_pointer>(pointer);
  }

  // find_index
  static iterator_data find_index_root(void_pointer pointer, size_type sz,
                                       size_type ht, size_type pos) {
    iterator_data it;
    it.pos = pos;

    if (ht < 2) {
      it.entry.leaf.pointer = nullptr;
      it.entry.leaf.index = 0;
      it.entry.segment = find_index_segment(cast_segment(pointer), sz, pos);
    } else
      it.entry = find_index_node(cast_node(pointer), ht, pos);

    return it;
  }

  static iterator_entry find_index_node(node_pointer pointer, size_type ht,
                                        size_type pos) {
    if (ht == 2) return find_index_leaf(pointer, pos);
    return find_index_branch(pointer, ht, pos);
  }

  static iterator_entry find_index_branch(node_pointer pointer, size_type ht,
                                          size_type pos) {
    while (true) {
      size_type index = 0;
      while (true) {
        auto size = pointer->sizes[index];
        if (pos < size) break;
        pos -= size;
        ++index;
      }

      auto child = cast_node(pointer->pointers[index]);
      --ht;
      if (ht == 2) return find_index_leaf(child, pos);

      pointer = child;
    }
  }

  static iterator_entry find_index_leaf(node_pointer pointer, size_type pos) {
    size_type index = 0;
    while (true) {
      auto size = pointer->sizes[index];
      if (pos < size) break;
      pos -= size;
      ++index;
    }

    iterator_entry entry;
    entry.leaf.pointer = pointer;
    entry.leaf.index = index;
    entry.segment = find_index_segment(cast_segment(pointer->pointers[index]),
                                       pointer->sizes[index], pos);
    return entry;
  }

  static segment_entry find_index_segment(element_pointer pointer, size_type sz,
                                          size_type pos) {
    segment_entry entry;
    entry.pointer = pointer;
    entry.index = pos;
    entry.length = sz;
    return entry;
  }

  // find_first
  static iterator_data find_first_root(void_pointer pointer, size_type sz,
                                       size_type ht) {
    iterator_data it;
    it.pos = 0;

    if (ht < 2) {
      it.entry.leaf.pointer = nullptr;
      it.entry.leaf.index = 0;
      it.entry.segment = find_first_segment(cast_segment(pointer), sz);
    } else
      it.entry = find_first_node(cast_node(pointer), ht);

    return it;
  }

  static iterator_entry find_first_node(node_pointer pointer, size_type ht) {
    if (ht == 2) return find_first_leaf(pointer);
    return find_first_branch(pointer, ht);
  }

  static iterator_entry find_first_branch(node_pointer pointer, size_type ht) {
    while (true) {
      auto child = cast_node(pointer->pointers[0]);
      --ht;
      if (ht == 2) return find_first_leaf(child);

      pointer = child;
    }
  }

  static iterator_entry find_first_leaf(node_pointer pointer) {
    iterator_entry entry;
    entry.leaf.pointer = pointer;
    entry.leaf.index = 0;
    entry.segment = find_first_segment(cast_segment(pointer->pointers[0]),
                                       pointer->sizes[0]);
    return entry;
  }

  static segment_entry find_first_segment(element_pointer pointer,
                                          size_type sz) {
    segment_entry entry;
    entry.pointer = pointer;
    entry.index = 0;
    entry.length = sz;
    return entry;
  }

  // find_last
  static iterator_data find_last_root(void_pointer pointer, size_type sz,
                                      size_type ht) {
    iterator_data it;
    it.pos = sz - 1;

    if (ht < 2) {
      it.entry.leaf.pointer = nullptr;
      it.entry.leaf.index = 0;
      it.entry.segment = find_last_segment(cast_segment(pointer), sz);
    } else
      it.entry = find_last_node(cast_node(pointer), ht);

    return it;
  }

  static iterator_entry find_last_node(node_pointer pointer, size_type ht) {
    if (ht == 2) return find_last_leaf(pointer);
    return find_last_branch(pointer, ht);
  }

  static iterator_entry find_last_branch(node_pointer pointer, size_type ht) {
    while (true) {
      auto index = pointer->length() - 1;
      auto child = cast_node(pointer->pointers[index]);
      --ht;
      if (ht == 2) return find_last_leaf(child);

      pointer = child;
    }
  }

  static iterator_entry find_last_leaf(node_pointer pointer) {
    iterator_entry entry;
    entry.leaf.pointer = pointer;
    entry.leaf.index = pointer->length() - 1;
    entry.segment =
        find_last_segment(cast_segment(pointer->pointers[entry.leaf.index]),
                          pointer->sizes[entry.leaf.index]);
    return entry;
  }

  static segment_entry find_last_segment(element_pointer pointer,
                                         size_type sz) {
    segment_entry entry;
    entry.pointer = pointer;
    entry.index = sz - 1;
    entry.length = sz;
    return entry;
  }

  // find_end
  static iterator_data find_end_root(void_pointer pointer, size_type sz,
                                     size_type ht) {
    iterator_data it;
    it.pos = sz;

    if (ht < 2) {
      it.entry.leaf.pointer = nullptr;
      it.entry.leaf.index = 0;
      it.entry.segment = find_end_segment(cast_segment(pointer), sz);
    } else
      it.entry = find_end_node(cast_node(pointer), ht);

    return it;
  }

  static iterator_entry find_end_node(node_pointer pointer, size_type ht) {
    if (ht == 2) return find_end_leaf(pointer);
    return find_end_branch(pointer, ht);
  }

  static iterator_entry find_end_branch(node_pointer pointer, size_type ht) {
    while (true) {
      auto index = pointer->length() - 1;
      auto child = cast_node(pointer->pointers[index]);

      --ht;
      if (ht == 2) return find_end_leaf(child);

      pointer = child;
    }
  }

  static iterator_entry find_end_leaf(node_pointer pointer) {
    iterator_entry entry;
    auto index = pointer->length() - 1;
    entry.leaf.pointer = pointer;
    entry.leaf.index = index;
    entry.segment = find_end_segment(cast_segment(pointer->pointers[index]),
                                     pointer->sizes[index]);
    return entry;
  }

  static segment_entry find_end_segment(element_pointer pointer, size_type sz) {
    segment_entry entry;
    entry.pointer = pointer;
    entry.index = sz;
    entry.length = sz;
    return entry;
  }

  // move_next
  static void move_next_iterator(iterator_data &it) {
    ++it.pos;
    move_next_segment(it.entry);
  }

  static void move_next_segment(iterator_entry &entry) {
    auto index = entry.segment.index;
    auto length = entry.segment.length;

    ++index;
    if (index != length) {
      entry.segment.index = index;
      return;
    }

    move_next_leaf(entry);
  }

  static void move_next_leaf(iterator_entry &entry) {
    auto pointer = entry.leaf.pointer;
    auto index = entry.leaf.index;

    // Special case for end iterator.
    if (pointer == nullptr) {
      entry.segment.index = entry.segment.length;
      return;
    }

    ++index;
    if (index != pointer->length()) {
      entry.leaf.index = index;
      entry.segment = find_first_segment(cast_segment(pointer->pointers[index]),
                                         pointer->sizes[index]);
      return;
    }

    move_next_branch(entry, pointer->parent_pointer, pointer->parent_index());
  }

  static void move_next_branch(iterator_entry &entry, node_pointer pointer,
                               size_type index) {
    size_type child_ht = 2;

    while (true) {
      // Special case for end iterator.
      if (pointer == nullptr) {
        entry.segment.index = entry.segment.length;
        return;
      }

      ++index;
      if (index != pointer->length()) {
        entry = find_first_node(cast_node(pointer->pointers[index]), child_ht);
        return;
      }

      index = pointer->parent_index();
      pointer = pointer->parent_pointer;
      ++child_ht;
    }
  }

  // move_prev
  static void move_prev_iterator(iterator_data &it) {
    --it.pos;
    move_prev_segment(it.entry);
  }

  static void move_prev_segment(iterator_entry &entry) {
    auto index = entry.segment.index;

    if (index != 0) {
      --index;
      entry.segment.index = index;
      return;
    }

    move_prev_leaf(entry);
  }

  static void move_prev_leaf(iterator_entry &entry) {
    auto pointer = entry.leaf.pointer;
    auto index = entry.leaf.index;

    if (index != 0) {
      --index;
      entry.leaf.index = index;
      entry.segment = find_last_segment(cast_segment(pointer->pointers[index]),
                                        pointer->sizes[index]);
      return;
    }

    move_prev_branch(entry, pointer->parent_pointer, pointer->parent_index());
  }

  static void move_prev_branch(iterator_entry &entry, node_pointer pointer,
                               size_type index) {
    size_type child_ht = 2;

    while (true) {
      if (index != 0) {
        entry =
            find_last_node(cast_node(pointer->pointers[index - 1]), child_ht);
        return;
      }

      index = pointer->parent_index();
      pointer = pointer->parent_pointer;
      ++child_ht;
    }
  }

  // move_count
  static void move_iterator_count(iterator_data &it, difference_type diff) {
    auto size = static_cast<size_type>(diff);
    it.pos += size;
    if (diff > 0)
      move_next_segment_count(it.entry, size);
    else if (diff < 0)
      move_prev_segment_count(it.entry, ~size + 1);
  }

  // move_next_count
  static void move_next_segment_count(iterator_entry &entry, size_type count) {
    auto index = entry.segment.index;
    auto length = entry.segment.length;

    index += count;
    if (index < length) {
      entry.segment.index = index;
      return;
    }

    move_next_leaf_count(entry, index - length);
  }

  static void move_next_leaf_count(iterator_entry &entry, size_type count) {
    auto pointer = entry.leaf.pointer;
    auto index = entry.leaf.index;

    // Special case for end iterator.
    if (pointer == nullptr) {
      entry.segment.index = entry.segment.length;
      return;
    }

    while (true) {
      ++index;
      if (index == pointer->length()) break;

      auto size = pointer->sizes[index];
      if (size > count) {
        entry.leaf.index = index;
        entry.segment = find_index_segment(
            cast_segment(pointer->pointers[index]), size, count);
        return;
      }
      count -= size;
    }

    move_next_branch_count(entry, pointer, pointer->parent_pointer,
                           pointer->parent_index(), count);
  }

  static void move_next_branch_count(iterator_entry &entry, node_pointer base,
                                     node_pointer pointer, size_type index,
                                     size_type count) {
    size_type child_ht = 2;

    while (true) {
      // Special case for end iterator.
      if (pointer == nullptr) {
        entry = find_end_node(base, child_ht);
        return;
      }

      while (true) {
        ++index;
        if (index == pointer->length()) break;

        auto size = pointer->sizes[index];
        if (size > count) {
          entry = find_index_node(cast_node(pointer->pointers[index]), child_ht,
                                  count);
          return;
        }
        count -= size;
      }

      base = pointer;
      index = pointer->parent_index();
      pointer = pointer->parent_pointer;
      ++child_ht;
    }
  }

  // move_prev_count
  static void move_prev_segment_count(iterator_entry &entry, size_type count) {
    auto index = entry.segment.index;

    if (index >= count) {
      index -= count;
      entry.segment.index = index;
      return;
    }

    move_prev_leaf_count(entry, count - index);
  }

  static void move_prev_leaf_count(iterator_entry &entry, size_type count) {
    auto pointer = entry.leaf.pointer;
    auto index = entry.leaf.index;

    while (true) {
      if (index == 0) break;
      --index;

      auto size = pointer->sizes[index];
      if (size >= count) {
        entry.leaf.index = index;
        entry.segment = find_index_segment(
            cast_segment(pointer->pointers[index]), size, size - count);
        return;
      }
      count -= size;
    }

    move_prev_branch_count(entry, pointer->parent_pointer,
                           pointer->parent_index(), count);
  }

  static void move_prev_branch_count(iterator_entry &entry,
                                     node_pointer pointer, size_type index,
                                     size_type count) {
    size_type child_ht = 2;

    while (true) {
      while (true) {
        if (index == 0) break;
        --index;

        auto size = pointer->sizes[index];
        if (size >= count) {
          entry = find_index_node(cast_node(pointer->pointers[index]), child_ht,
                                  size - count);
          return;
        }
        count -= size;
      }

      index = pointer->parent_index();
      pointer = pointer->parent_pointer;
      ++child_ht;
    }
  }
#endif  // BOOST_CONTAINER_DOXYGEN_INVOKED

  /// \brief A template class used for const and non-const iterators for
  /// segmented_tree_seq.
  ///
  /// \tparam Pointer A const or non-const pointer.
  /// \tparam Reference A const or non-const reference.
  template <typename Pointer, typename Reference>
  class iterator_t {
    template <typename, typename, std::size_t, std::size_t>
    friend class boost::container::segmented_tree_seq;

    template <typename, typename>
    friend class iterator_t;

   private:
    iterator_data it_;
    iterator_t(iterator_data it) : it_(it) {}

   public:
    /// \brief The iterator category type.
    using iterator_category = std::random_access_iterator_tag;
    /// \brief The value type.
    using value_type = T;
    /// \brief The difference type.
    using difference_type =
        typename std::pointer_traits<VoidPointer>::difference_type;
    /// \brief The pointer type.
    using pointer = Pointer;
    /// \brief The iterator reference type.
    using reference = Reference;

    /// \brief Default constructs an iterator_t from another.
    ///
    /// \par Complexity
    ///   Constant.
    iterator_t() = default;

    /// \brief Copy constructs an iterator_t from other.
    ///
    /// \par Complexity
    ///   Constant.
    iterator_t(iterator_t const &other) = default;

    /// \brief Copy assigns from other.
    ///
    /// \par Complexity
    ///   Constant.
    iterator_t &operator=(iterator_t const &other) = default;

    /// \brief Copy constructs a const iterator from non-const iterator other.
    ///
    /// \par Complexity
    ///   Constant.
    template <typename P, typename R,
              typename = typename std::enable_if<
                  std::is_convertible<P, pointer>::value>::type>
    iterator_t(iterator_t<P, R> const &other) : it_(other.it_) {}

    /// \brief Returns the index into the segment that the iterator points to.
    ///
    /// \par Complexity
    ///   Constant.
    ///
    /// \note
    ///   Non-standard extension.
    size_type index() const { return it_.entry.segment.index; }

    /// \brief Returns the beginning of the segment that the iterator points to.
    ///
    /// \par Complexity
    ///   Constant.
    ///
    /// \note
    ///   Non-standard extension.
    pointer begin() const { return it_.entry.segment.pointer; }

    /// \brief Returns the end of the segment that the iterator points to.
    ///
    /// \par Complexity
    ///   Constant.
    ///
    /// \note
    ///   Non-standard extension.
    pointer end() const { return begin() + it_.entry.segment.length; }

    /// \brief Returns a pointer to the current element.
    ///
    /// \par Complexity
    ///   Constant.
    pointer operator->() const { return begin() + index(); }

    /// \brief Returns a reference to the current element.
    ///
    /// \par Complexity
    ///   Constant.
    reference operator*() const { return begin()[index()]; }

    /// \brief Move the iterator forward 1 element.
    ///
    /// \par Complexity
    ///   Constant amortized.
    iterator_t &operator++() {
      move_next_iterator(it_);
      return *this;
    }

    /// \brief Move the iterator backward 1 element.
    ///
    /// \par Complexity
    ///   Constant amortized.
    iterator_t &operator--() {
      move_prev_iterator(it_);
      return *this;
    }

    /// \brief Return a copy of the iterator moved forward 1 element.
    ///
    /// \par Complexity
    ///   Constant amortized.
    iterator_t operator++(int) {
      auto copy = it_;
      move_next_iterator(it_);
      return copy;
    }

    /// \brief Return a copy of the iterator moved backward 1 element.
    ///
    /// \par Complexity
    ///   Constant amortized.
    iterator_t operator--(int) {
      auto copy = it_;
      move_prev_iterator(it_);
      return copy;
    }

    /// \brief Move the iterator forward diff elements.
    ///
    /// \par Complexity
    ///   Logarithmic amortized in the absolute value of diff.
    iterator_t &operator+=(difference_type diff) {
      move_iterator_count(it_, diff);
      return *this;
    }

    /// \brief Move the iterator backward diff elements.
    ///
    /// \par Complexity
    ///   Logarithmic amortized in the absolute value of diff.
    iterator_t &operator-=(difference_type diff) {
      move_iterator_count(it_, -diff);
      return *this;
    }

    /// \brief Return a copy of the iterator moved forward diff elements.
    ///
    /// \par Complexity
    ///   Logarithmic amortized in the absolute value of diff.
    iterator_t operator+(difference_type diff) const {
      auto copy = it_;
      move_iterator_count(copy, diff);
      return copy;
    }

    /// \brief Return a copy of the iterator moved backward diff elements.
    ///
    /// \par Complexity
    ///   Logarithmic amortized in the absolute value of diff.
    iterator_t operator-(difference_type diff) const {
      auto copy = it_;
      move_iterator_count(copy, -diff);
      return copy;
    }

    /// \brief Return a distance between the specified iterator.
    ///
    /// \par Complexity
    ///   Constant.
    difference_type operator-(iterator_t const &other) const {
      return it_.pos > other.it_.pos
                 ? static_cast<difference_type>(it_.pos - other.it_.pos)
                 : -static_cast<difference_type>(other.it_.pos - it_.pos);
    }

    /// \brief Move the iterator to the last element of the previous segment.
    ///
    /// \par Complexity
    ///   Constant amortized.
    ///
    /// \note
    ///   Non-standard extension.
    iterator_t &move_before_segment() {
      it_.pos -= it_.entry.segment.index + 1;
      move_prev_leaf(it_.entry);
      return *this;
    }

    /// \brief Move the iterator to the last element of the previous segment and
    ///        then move backward count elements.
    ///
    /// \par Complexity
    ///   Logarithmic amortized in count.
    ///
    /// \note
    ///   Non-standard extension.
    iterator_t &move_before_segment(size_type count) {
      it_.pos -= it_.entry.segment.index + 1 - count;
      move_prev_leaf_count(it_.entry, count);
      return *this;
    }

    /// \brief Move the iterator to the first element of the next segment.
    ///
    /// \par Complexity
    ///   Constant amortized.
    ///
    /// \note
    ///   Non-standard extension.
    iterator_t &move_after_segment() {
      it_.pos += it_.entry.segment.length - it_.entry.segment.index;
      move_next_leaf(it_.entry);
      return *this;
    }

    /// \brief Move the iterator to the first element of the next segment and
    ///        then move forward count elements.
    ///
    /// \par Complexity
    ///   Logarithmic amortized in count.
    ///
    /// \note
    ///   Non-standard extension.
    iterator_t &move_after_segment(size_type count) {
      it_.pos += it_.entry.segment.length - it_.entry.segment.index + count;
      move_next_leaf_count(it_.entry, count);
      return *this;
    }

    /// \brief Return a copy of the iterator moved to the last element of the
    ///        previous segment.
    ///
    /// \par Complexity
    ///   Constant amortized.
    ///
    /// \note
    ///   Non-standard extension.
    iterator_t before_segment() const {
      auto copy = it_;
      move_before_segment(copy);
      return copy;
    }

    /// \brief Return a copy of the iterator moved to the last element of the
    ///        previous segment and then move backward count elements.
    ///
    /// \par Complexity
    ///   Logarithmic amortized in count.
    ///
    /// \note
    ///   Non-standard extension.
    iterator_t after_segment() const {
      auto copy = it_;
      move_after_segment(copy);
      return copy;
    }

    /// \brief Return a copy of the iterator moved to the last element of the
    ///        previous segment.
    ///
    /// \par Complexity
    ///   Constant amortized.
    ///
    /// \note
    ///   Non-standard extension.
    iterator_t before_segment(size_type count) const {
      auto copy = it_;
      move_before_segment_count(copy, count);
      return copy;
    }

    /// \brief Return a copy of the iterator to the first element of the next
    ///        segment and then move forward count elements.
    ///
    /// \par Complexity
    ///   Logarithmic amortized in count.
    ///
    /// \note
    ///   Non-standard extension.
    iterator_t after_segment(size_type count) const {
      auto copy = it_;
      move_after_segment_count(copy, count);
      return copy;
    }

    /// \brief Return false if both iterators point to the same element. Return
    ///        true otherwise.
    ///
    /// \par Complexity
    ///   Constant.
    bool operator!=(iterator_t const &other) const {
      return it_.pos != other.it_.pos;
    }

    /// \brief Return true if both iterators point to the same element. Return
    ///        false otherwise.
    ///
    /// \par Complexity
    ///   Constant.
    bool operator==(iterator_t const &other) const {
      return it_.pos == other.it_.pos;
    }

    /// \brief Return true if *this points to an element before other. Return
    ///        false otherwise.
    ///
    /// \par Complexity
    ///   Constant.
    bool operator<(iterator_t const &other) const {
      return it_.pos < other.it_.pos;
    }

    /// \brief Return true if *this points to an element after other. Return
    ///        false otherwise.
    ///
    /// \par Complexity
    ///   Constant.
    bool operator>(iterator_t const &other) const {
      return it_.pos > other.it_.pos;
    }

    /// \brief Return true if *this points to the same element or before other.
    ///        Return false otherwise.
    ///
    /// \par Complexity
    ///   Constant.
    bool operator<=(iterator_t const &other) const {
      return it_.pos <= other.it_.pos;
    }

    /// \brief Return true if *this points to the same element or after other.
    ///        Return false otherwise.
    ///
    /// \par Complexity
    ///   Constant.
    bool operator>=(iterator_t const &other) const {
      return it_.pos >= other.it_.pos;
    }
  };
};
}

/// \brief A segmented_tree_seq is a sequence container that provides efficient
///        random access insert and erase.
///
/// \tparam T The type of object to be stored
/// \tparam Allocator The type of the allocator used for all memory management
/// \tparam segment_target Size in bytes to try to use for object nodes
/// \tparam base_target Size in bytes to try to use for index nodes
template <typename T, typename Allocator, std::size_t segment_target,
          std::size_t base_target>
class segmented_tree_seq {
 private:
  using element_traits = typename std::allocator_traits<Allocator>;
  using static_traits = segmented_tree_seq_detail::static_traits_t<
      T, typename element_traits::void_pointer,
      typename element_traits::size_type, segment_target, base_target>;

  using element_pointer = typename static_traits::element_pointer;
  using void_pointer = typename static_traits::void_pointer;
  using node_pointer = typename static_traits::node_pointer;
  using node_type = typename static_traits::node;
  using iterator_data = typename static_traits::iterator_data;
  using iterator_entry = typename static_traits::iterator_entry;
  using leaf_entry = typename static_traits::leaf_entry;
  template <typename Pointer, typename Reference>
  using iterator_t =
      typename static_traits::template iterator_t<Pointer, Reference>;

  using node_allocator =
      typename element_traits::template rebind_alloc<node_type>;
  using node_traits =
      typename element_traits::template rebind_traits<node_type>;

  static constexpr bool is_trivial_ = std::is_trivial<T>::value;

 public:
  /// \brief The type of elements stored in the container.
  using value_type = typename element_traits::value_type;
  /// \brief The allocator type used by the container.
  using allocator_type = Allocator;
  /// \brief The unsigned integral type used by the container.
  using size_type = typename element_traits::size_type;
  /// \brief The pointers difference type.
  using difference_type = typename element_traits::difference_type;
  /// \brief The value reference type.
  using reference = value_type &;
  /// \brief The value const reference type.
  using const_reference = value_type const &;
  /// \brief The pointer type.
  using pointer = typename element_traits::pointer;
  /// \brief The const pointer type.
  using const_pointer = typename element_traits::const_pointer;
  /// \brief The iterator type.
  using iterator = iterator_t<pointer, T &>;
  /// \brief The const iterator type.
  using const_iterator = iterator_t<const_pointer, T const &>;
  /// \brief The reverse iterator type.
  using reverse_iterator = std::reverse_iterator<iterator>;
  /// \brief The const reverse iterator.
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

 private:
  // private data
  std::tuple<void_pointer, size_type, size_type, allocator_type, node_allocator>
      data_;

  // getters
  void_pointer &get_root() { return std::get<0>(data_); }
  void_pointer const &get_root() const { return std::get<0>(data_); }
  size_type &get_size() { return std::get<1>(data_); }
  size_type const &get_size() const { return std::get<1>(data_); }
  size_type &get_height() { return std::get<2>(data_); }
  size_type const &get_height() const { return std::get<2>(data_); }
  allocator_type &get_element_allocator() { return std::get<3>(data_); }
  allocator_type const &get_element_allocator() const {
    return std::get<3>(data_);
  }
  node_allocator &get_node_allocator() { return std::get<4>(data_); }
  node_allocator const &get_node_allocator() const {
    return std::get<4>(data_);
  }

  // construct
  template <typename... Args>
  void construct_element(element_pointer pointer, size_type index,
                         Args &&... args) {
    element_traits::construct(get_element_allocator(),
                              std::addressof(pointer[index]),
                              std::forward<Args>(args)...);
  }

  // destroy
  void destroy_element(element_pointer pointer, size_type index) {
    element_traits::destroy(get_element_allocator(),
                            std::addressof(pointer[index]));
  }

  // allocate
  element_pointer allocate_segment() {
    return element_traits::allocate(get_element_allocator(),
                                    static_traits::segment_max);
  }

  node_pointer allocate_node() {
    return node_traits::allocate(get_node_allocator(), 1);
  }

  // deallocate
  void deallocate_segment(element_pointer pointer) {
    element_traits::deallocate(get_element_allocator(), pointer,
                               static_traits::segment_max);
  }

  void deallocate_node(node_pointer pointer) {
    node_traits::deallocate(get_node_allocator(), pointer, 1);
  }

  // purge
  void purge() { purge_root(get_root(), get_size(), get_height()); }

  void purge_segment(element_pointer pointer, size_type sz) {
    if (!is_trivial_)
      for (size_type i = 0, e = sz; i != e; ++i) destroy_element(pointer, i);
    deallocate_segment(pointer);
  }

  void purge_leaf(node_pointer pointer) {
    for (size_type i = 0, e = pointer->length(); i != e; ++i)
      purge_segment(static_traits::cast_segment(pointer->pointers[i]),
                    pointer->sizes[i]);
    deallocate_node(pointer);
  }

  void purge_node(node_pointer pointer, size_type ht) {
    if (ht == 2)
      purge_leaf(pointer);
    else
      purge_branch(pointer, ht);
  }

  void purge_branch(node_pointer pointer, size_type ht) {
    for (size_type i = 0, e = pointer->length(); i != e; ++i)
      purge_node(static_traits::cast_node(pointer->pointers[i]), ht - 1);
    deallocate_node(pointer);
  }

  void purge_root(void_pointer pointer, size_type sz, size_type ht) {
    if (ht < 2)
      purge_segment(static_traits::cast_segment(pointer), sz);
    else
      purge_node(static_traits::cast_node(pointer), ht);
  }

  // move_single
  size_type move_single_segment(element_pointer source, size_type source_index,
                                element_pointer dest, size_type dest_index) {
    if (is_trivial_)
      dest[dest_index] = source[source_index];
    else {
      construct_element(dest, dest_index, std::move(source[source_index]));
      destroy_element(source, source_index);
    }
    return 1;
  }

  size_type move_single_leaf(node_pointer source, size_type source_index,
                             node_pointer dest, size_type dest_index) {
    auto sz = source->sizes[source_index];
    dest->sizes[dest_index] = sz;
    dest->pointers[dest_index] = source->pointers[source_index];
    return sz;
  }

  size_type move_single_branch(node_pointer source, size_type source_index,
                               node_pointer dest, size_type dest_index) {
    auto sz = source->sizes[source_index];
    dest->sizes[dest_index] = sz;
    auto child = static_traits::cast_node(source->pointers[source_index]);
    child->parent_pointer = dest;
    child->parent_index(dest_index);
    dest->pointers[dest_index] = child;
    return sz;
  }

  // move_range
  size_type move_range_segment(element_pointer source, size_type source_index,
                               element_pointer dest, size_type dest_index,
                               size_type count) {
    if (is_trivial_)
      std::memcpy(std::addressof(dest[dest_index]),
                  std::addressof(source[source_index]), count * sizeof(T));
    else {
      auto from = source_index;
      auto last = source_index + count;
      auto to = dest_index;

      while (from != last) {
        construct_element(dest, to, std::move(source[from]));
        destroy_element(source, from);
        ++from;
        ++to;
      }
    }
    return count;
  }

  size_type move_range_leaf(node_pointer source, size_type source_index,
                            node_pointer dest, size_type dest_index,
                            size_type count) {
    size_type copy_size = 0;
    auto from = source_index;
    auto last = source_index + count;
    auto to = dest_index;

    while (from != last) {
      auto sz = source->sizes[from];
      dest->sizes[to] = sz;
      dest->pointers[to] = source->pointers[from];
      copy_size += sz;
      ++from;
      ++to;
    }

    return copy_size;
  }

  size_type move_range_branch(node_pointer source, size_type source_index,
                              node_pointer dest, size_type dest_index,
                              size_type count) {
    size_type copy_size = 0;
    auto from = source_index;
    auto last = source_index + count;
    auto to = dest_index;

    while (from != last) {
      auto sz = source->sizes[from];
      dest->sizes[to] = sz;
      auto child = static_traits::cast_node(source->pointers[from]);
      child->parent_pointer = dest;
      child->parent_index(to);
      dest->pointers[to] = child;
      copy_size += sz;
      ++from;
      ++to;
    }

    return copy_size;
  }

  // move_forward
  void move_forward_segment(element_pointer pointer, size_type length,
                            size_type index, size_type distance) {
    if (is_trivial_)
      std::memmove(std::addressof(pointer[index + distance]),
                   std::addressof(pointer[index]),
                   (length - index) * sizeof(T));
    else {
      auto first = index;
      auto from = length;
      auto to = length + distance;

      while (first != from) {
        --from;
        --to;
        construct_element(pointer, to, std::move(pointer[from]));
        destroy_element(pointer, from);
      }
    }
  }

  void move_forward_leaf(node_pointer pointer, size_type length,
                         size_type index, size_type distance) {
    auto first = index;
    auto from = length;
    auto to = length + distance;

    while (first != from) {
      --from;
      --to;
      pointer->sizes[to] = pointer->sizes[from];
      pointer->pointers[to] = pointer->pointers[from];
    }
  }

  void move_forward_branch(node_pointer pointer, size_type length,
                           size_type index, size_type distance) {
    auto first = index;
    auto from = length;
    auto to = length + distance;

    while (first != from) {
      --from;
      --to;
      pointer->sizes[to] = pointer->sizes[from];
      auto child = static_traits::cast_node(pointer->pointers[from]);
      child->parent_index(to);
      pointer->pointers[to] = child;
    }
  }

  // move_backward
  void move_backward_segment(element_pointer pointer, size_type length,
                             size_type index, size_type distance) {
    if (is_trivial_)
      std::memmove(std::addressof(pointer[index]),
                   std::addressof(pointer[index + distance]),
                   (length - index) * sizeof(T));
    else {
      auto from = index + distance;
      auto to = index;
      auto last = length;

      while (to != last) {
        destroy_element(pointer, to);
        construct_element(pointer, to, std::move(pointer[from]));
        ++from;
        ++to;
      }
    }
  }

  void move_backward_leaf(node_pointer pointer, size_type length,
                          size_type index, size_type distance) {
    auto from = index + distance;
    auto to = index;
    auto last = length;

    while (to != last) {
      pointer->sizes[to] = pointer->sizes[from];
      pointer->pointers[to] = pointer->pointers[from];
      ++from;
      ++to;
    }
  }

  void move_backward_branch(node_pointer pointer, size_type length,
                            size_type index, size_type distance) {
    auto from = index + distance;
    auto to = index;
    auto last = length;

    while (to != last) {
      pointer->sizes[to] = pointer->sizes[from];
      auto child = static_traits::cast_node(pointer->pointers[from]);
      child->parent_index(to);
      pointer->pointers[to] = child;
      ++from;
      ++to;
    }
  }

  // copy_single
  template <typename... Args>
  size_type copy_single_segment(element_pointer pointer, size_type index,
                                Args &&... args) {
    construct_element(pointer, index, std::forward<Args>(args)...);
    return 1;
  }

  size_type copy_single_leaf(node_pointer pointer, size_type index,
                             element_pointer child_pointer,
                             size_type child_size) {
    pointer->sizes[index] = child_size;
    pointer->pointers[index] = child_pointer;
    return child_size;
  }

  size_type copy_single_branch(node_pointer pointer, size_type index,
                               node_pointer child_pointer,
                               size_type child_size) {
    child_pointer->parent_pointer = pointer;
    child_pointer->parent_index(index);
    pointer->sizes[index] = child_size;
    pointer->pointers[index] = child_pointer;
    return child_size;
  }

  // purge_single

  // purge_range
  size_type purge_range_segment(element_pointer pointer, size_type index,
                                size_type count) {
    if (!is_trivial_) {
      auto from = index;
      auto last = index + count;

      while (from != last) {
        destroy_element(pointer, from);
        ++from;
      }
    }
    return count;
  }

  size_type purge_range_leaf(node_pointer pointer, size_type index,
                             size_type count) {
    auto from = index;
    auto last = index + count;
    size_type erase_size = 0;

    while (from != last) {
      auto sz = pointer->sizes[from];
      purge_segment(static_traits::cast_segment(pointer->pointers[from]), sz);
      erase_size += sz;
      ++from;
    }

    return erase_size;
  }

  size_type purge_range_branch(node_pointer pointer, size_type index,
                               size_type count, size_type ht) {
    auto from = index;
    auto last = index + count;
    size_type erase_size = 0;

    while (from != last) {
      auto sz = pointer->sizes[from];
      purge_node(static_traits::cast_node(pointer->pointers[from]), sz, ht - 1);
      erase_size += sz;
      ++from;
    }

    return erase_size;
  }

  // update_sizes
  void update_sizes(node_pointer pointer, size_type index, size_type sz) {
    while (pointer != nullptr) {
      pointer->sizes[index] += sz;
      index = pointer->parent_index();
      pointer = pointer->parent_pointer;
    }

    get_size() += sz;
  }

  void increment_sizes(node_pointer pointer, size_type index,
                       std::size_t by = 1) {
    update_sizes(pointer, index, by);
  }

  void decrement_sizes(node_pointer pointer, size_type index,
                       std::size_t by = 1) {
    update_sizes(pointer, index, ~by + 1);
  }

  // alloc_nodes_single
  node_pointer alloc_nodes_single(node_pointer pointer,
                                  element_pointer segment_alloc) {
    node_pointer alloc = nullptr;
    try {
      while (true) {
        if (pointer == nullptr) {
          auto temp = allocate_node();
          temp->parent_pointer = alloc;
          alloc = temp;
          return alloc;
        }

        if (pointer->length() != static_traits::base_max) return alloc;

        auto temp = allocate_node();
        temp->parent_pointer = alloc;
        alloc = temp;

        pointer = pointer->parent_pointer;
      }
    } catch (...) {
      deallocate_segment(segment_alloc);

      while (alloc != nullptr) {
        auto temp = alloc->parent_pointer;
        deallocate_node(alloc);
        alloc = temp;
      }
      throw;
    }
  }

  // reserve_single
  iterator_data reserve_single_iterator(iterator_data const &it) {
    auto res = it;
    reserve_single_segment(res.entry);
    return res;
  }

  void reserve_single_segment(iterator_entry &entry) {
    auto pointer = entry.segment.pointer;
    auto index = entry.segment.index;
    auto length = entry.segment.length;
    auto parent_pointer = entry.leaf.pointer;
    auto parent_index = entry.leaf.index;

    if (pointer == nullptr) {
      auto alloc = allocate_segment();
      get_root() = alloc;
      get_size() = 1;
      get_height() = 1;
      entry.segment.pointer = alloc;
      entry.segment.length = 1;
      return;
    }

    if (length != static_traits::segment_max) {
      move_forward_segment(pointer, length, index, 1);
      ++entry.segment.length;
      increment_sizes(parent_pointer, parent_index);
      return;
    }

    auto alloc = allocate_segment();
    auto leaf_alloc = alloc_nodes_single(parent_pointer, alloc);

    constexpr auto sum = static_traits::segment_max + 1;
    constexpr auto pointer_length = sum / 2;
    constexpr auto alloc_length = sum - pointer_length;

    if (index < pointer_length) {
      auto left_index = pointer_length - 1;
      move_range_segment(pointer, left_index, alloc, 0, alloc_length);
      move_forward_segment(pointer, left_index, index, 1);
      entry.segment.length = pointer_length;
    } else {
      auto new_index = index - pointer_length;
      auto move_length = length - index;
      move_range_segment(pointer, pointer_length, alloc, 0, new_index);
      move_range_segment(pointer, index, alloc, new_index + 1, move_length);
      entry.segment.length = alloc_length;
      entry.segment.pointer = alloc;
      entry.segment.index = new_index;
      ++entry.leaf.index;
    }

    reserve_single_leaf(entry, pointer, parent_pointer, parent_index + 1,
                        leaf_alloc, alloc, alloc_length);
  }

  void reserve_single_leaf(iterator_entry &entry, element_pointer base,
                           node_pointer pointer, size_type index,
                           node_pointer alloc, element_pointer child_pointer,
                           size_type child_size) {
    if (pointer == nullptr) {
      alloc->parent_pointer = nullptr;
      alloc->parent_index(0);
      alloc->length(2);
      copy_single_leaf(alloc, 0, base, get_size() - child_size + 1);
      copy_single_leaf(alloc, 1, child_pointer, child_size);
      get_root() = alloc;
      get_height() = 2;
      ++get_size();
      entry.leaf.pointer = alloc;
      return;
    }

    pointer->sizes[index - 1] -= child_size - 1;

    auto length = pointer->length();
    if (length != static_traits::base_max) {
      move_forward_leaf(pointer, length, index, 1);
      copy_single_leaf(pointer, index, child_pointer, child_size);
      pointer->length(length + 1);
      increment_sizes(pointer->parent_pointer, pointer->parent_index());
      return;
    }

    auto next_alloc = alloc->parent_pointer;
    constexpr auto sum = static_traits::base_max + 1;
    constexpr auto pointer_length = sum / 2;
    constexpr auto alloc_length = sum - pointer_length;

    size_type alloc_size = 0;
    if (index < pointer_length) {
      auto left_index = pointer_length - 1;
      alloc_size +=
          move_range_leaf(pointer, left_index, alloc, 0, alloc_length);
      move_forward_leaf(pointer, left_index, index, 1);
      copy_single_leaf(pointer, index, child_pointer, child_size);
    } else {
      auto new_index = index - pointer_length;
      auto move_length = length - index;
      alloc_size +=
          move_range_leaf(pointer, pointer_length, alloc, 0, new_index);
      alloc_size +=
          move_range_leaf(pointer, index, alloc, new_index + 1, move_length);
      alloc_size +=
          copy_single_leaf(alloc, new_index, child_pointer, child_size);
    }

    pointer->length(pointer_length);
    alloc->length(alloc_length);

    if (entry.leaf.index >= pointer_length) {
      entry.leaf.pointer = alloc;
      entry.leaf.index -= pointer_length;
    }

    reserve_single_branch(pointer, pointer->parent_pointer,
                          pointer->parent_index() + 1, next_alloc, alloc,
                          alloc_size);
  }

  void reserve_single_branch(node_pointer base, node_pointer pointer,
                             size_type index, node_pointer alloc,
                             node_pointer child_pointer, size_type child_size) {
    while (true) {
      if (pointer == nullptr) {
        alloc->parent_pointer = nullptr;
        alloc->parent_index(0);
        alloc->length(2);
        copy_single_branch(alloc, 0, base, get_size() - child_size + 1);
        copy_single_branch(alloc, 1, child_pointer, child_size);
        get_root() = alloc;
        ++get_height();
        ++get_size();
        return;
      }

      pointer->sizes[index - 1] -= child_size - 1;

      auto length = pointer->length();
      if (length != static_traits::base_max) {
        move_forward_branch(pointer, length, index, 1);
        copy_single_branch(pointer, index, child_pointer, child_size);
        pointer->length(length + 1);
        increment_sizes(pointer->parent_pointer, pointer->parent_index());
        return;
      }

      auto next_alloc = alloc->parent_pointer;
      constexpr auto sum = static_traits::base_max + 1;
      constexpr auto pointer_length = sum / 2;
      constexpr auto alloc_length = sum - pointer_length;

      size_type alloc_size = 0;
      if (index < pointer_length) {
        auto left_index = pointer_length - 1;
        alloc_size +=
            move_range_branch(pointer, left_index, alloc, 0, alloc_length);
        move_forward_branch(pointer, left_index, index, 1);
        copy_single_branch(pointer, index, child_pointer, child_size);
      } else {
        auto new_index = index - pointer_length;
        auto move_length = length - index;
        alloc_size +=
            move_range_branch(pointer, pointer_length, alloc, 0, new_index);
        alloc_size += move_range_branch(pointer, index, alloc, new_index + 1,
                                        move_length);
        alloc_size +=
            copy_single_branch(alloc, new_index, child_pointer, child_size);
      }

      pointer->length(pointer_length);
      alloc->length(alloc_length);

      child_pointer = alloc;
      child_size = alloc_size;
      base = pointer;
      index = pointer->parent_index() + 1;
      pointer = pointer->parent_pointer;
      alloc = next_alloc;
    }
  }

  // erase_single
  iterator_data erase_single_iterator(iterator_data const &it) {
    auto res = it;
    destroy_element(res.entry.segment.pointer, res.entry.segment.index);
    erase_single_segment(res.entry);
    if (res.entry.segment.index == res.entry.segment.length)
      static_traits::move_next_leaf(res.entry);
    return res;
  }

  void erase_single_segment(iterator_entry &entry) {
    auto pointer = entry.segment.pointer;
    auto index = entry.segment.index;
    auto length = entry.segment.length;
    auto parent_pointer = entry.leaf.pointer;
    auto parent_index = entry.leaf.index;

    if (length == 1 &&
        (static_traits::segment_min != 1 || parent_pointer == nullptr)) {
      deallocate_segment(pointer);
      get_root() = nullptr;
      get_size() = 0;
      get_height() = 0;
      entry.segment.pointer = nullptr;
      entry.segment.index = 0;
      entry.segment.length = 0;
      return;
    }

    if (length-- != static_traits::segment_min || parent_pointer == nullptr) {
      move_backward_segment(pointer, length, index, 1);
      entry.segment.length = length;
      decrement_sizes(parent_pointer, parent_index);
      return;
    }

    constexpr auto merge_size = static_traits::segment_min * 2 - 1;
    auto pointers = &parent_pointer->pointers[0];
    auto sizes = &parent_pointer->sizes[0];

    size_type erase_index;
    if (parent_index != 0) {
      auto prev_index = parent_index - 1;
      auto prev_pointer = static_traits::cast_segment(pointers[prev_index]);
      auto prev_length = sizes[prev_index];

      if (prev_length != static_traits::segment_min) {
        --prev_length;
        move_forward_segment(pointer, index, 0, 1);
        move_single_segment(prev_pointer, prev_length, pointer, 0);
        sizes[prev_index] = prev_length;
        ++entry.segment.index;
        decrement_sizes(parent_pointer->parent_pointer,
                        parent_pointer->parent_index());
        return;
      }

      move_range_segment(pointer, 0, prev_pointer, prev_length, index);
      move_range_segment(pointer, index + 1, prev_pointer, prev_length + index,
                         length - index);
      sizes[prev_index] = merge_size;
      erase_index = parent_index;
      entry.segment.pointer = prev_pointer;
      entry.segment.length = merge_size;
      entry.segment.index += static_traits::segment_min;
      --entry.leaf.index;
    }

    else {
      auto next_index = parent_index + 1;
      auto next_pointer = static_traits::cast_segment(pointers[next_index]);
      auto next_length = sizes[next_index];

      if (next_length != static_traits::segment_min) {
        --next_length;
        move_backward_segment(pointer, length, index, 1);
        move_single_segment(next_pointer, 0, pointer, length);
        move_backward_segment(next_pointer, next_length, 0, 1);
        sizes[next_index] = next_length;
        decrement_sizes(parent_pointer->parent_pointer,
                        parent_pointer->parent_index());
        return;
      }

      move_backward_segment(pointer, length, index, 1);
      move_range_segment(next_pointer, 0, pointer, length, next_length);
      sizes[parent_index] = merge_size;
      erase_index = next_index;
      entry.segment.length = merge_size;
    }

    erase_single_leaf(entry.leaf, parent_pointer, erase_index);
  }

  void erase_single_leaf(leaf_entry &entry, node_pointer pointer,
                         size_type index) {
    deallocate_segment(static_traits::cast_segment(pointer->pointers[index]));

    auto parent_pointer = pointer->parent_pointer;
    auto parent_index = pointer->parent_index();
    auto length = pointer->length();

    if (length == 2 &&
        (static_traits::base_min != 2 || parent_pointer == nullptr)) {
      auto other = pointer->pointers[index ^ 1];
      deallocate_node(pointer);
      get_root() = other;
      --get_size();
      get_height() = 1;
      entry.pointer = nullptr;
      entry.index = 0;
      return;
    }

    if (length-- != static_traits::base_min || parent_pointer == nullptr) {
      move_backward_leaf(pointer, length, index, 1);
      pointer->length(length);
      decrement_sizes(parent_pointer, parent_index);
      return;
    }

    auto pointers = &parent_pointer->pointers[0];
    auto sizes = &parent_pointer->sizes[0];

    size_type erase_index;
    if (parent_index != 0) {
      auto prev_index = parent_index - 1;
      auto prev_pointer = static_traits::cast_node(pointers[prev_index]);
      auto prev_length = prev_pointer->length();

      if (prev_length != static_traits::base_min) {
        --prev_length;
        move_forward_leaf(pointer, index, 0, 1);
        auto sz = move_single_leaf(prev_pointer, prev_length, pointer, 0);
        sizes[prev_index] -= sz;
        sizes[parent_index] += sz - 1;
        prev_pointer->length(prev_length);
        ++entry.index;
        decrement_sizes(parent_pointer->parent_pointer,
                        parent_pointer->parent_index());

        return;
      }

      auto sz = move_range_leaf(pointer, 0, prev_pointer, prev_length, index);
      sz += move_range_leaf(pointer, index + 1, prev_pointer,
                            prev_length + index, length - index);
      prev_pointer->length(prev_length + length);
      sizes[prev_index] += sz;
      erase_index = parent_index;
      entry.pointer = prev_pointer;
      entry.index += prev_length;
    }

    else {
      auto next_index = parent_index + 1;
      auto next_pointer = static_traits::cast_node(pointers[next_index]);
      auto next_length = next_pointer->length();

      if (next_length != static_traits::base_min) {
        --next_length;
        move_backward_leaf(pointer, length, index, 1);
        auto sz = move_single_leaf(next_pointer, 0, pointer, length);
        move_backward_leaf(next_pointer, next_length, 0, 1);
        sizes[next_index] -= sz;
        sizes[parent_index] += sz - 1;
        next_pointer->length(next_length);
        decrement_sizes(parent_pointer->parent_pointer,
                        parent_pointer->parent_index());

        return;
      }

      move_backward_leaf(pointer, length, index, 1);
      auto sz = move_range_leaf(next_pointer, 0, pointer, length, next_length);
      pointer->length(length + next_length);
      sizes[parent_index] += sz - 1;
      erase_index = next_index;
    }

    erase_single_branch(parent_pointer, erase_index);
  }

  void erase_single_branch(node_pointer pointer, size_type index) {
    while (true) {
      deallocate_node(static_traits::cast_node(pointer->pointers[index]));

      auto parent_pointer = pointer->parent_pointer;
      auto parent_index = pointer->parent_index();
      auto length = pointer->length();

      if (length == 2 &&
          (static_traits::base_min != 2 || parent_pointer == nullptr)) {
        auto other = static_traits::cast_node(pointer->pointers[index ^ 1]);
        deallocate_node(pointer);
        get_root() = other;
        other->parent_pointer = nullptr;
        other->parent_index(0);
        --get_size();
        --get_height();

        return;
      }

      if (length-- != static_traits::base_min || parent_pointer == nullptr) {
        move_backward_branch(pointer, length, index, 1);
        pointer->length(length);
        decrement_sizes(parent_pointer, parent_index);
        return;
      }

      auto pointers = &parent_pointer->pointers[0];
      auto sizes = &parent_pointer->sizes[0];

      size_type erase_index;
      if (parent_index != 0) {
        auto prev_index = parent_index - 1;
        auto prev_pointer = static_traits::cast_node(pointers[prev_index]);
        auto prev_length = prev_pointer->length();

        if (prev_length != static_traits::base_min) {
          --prev_length;
          move_forward_branch(pointer, index, 0, 1);
          auto sz = move_single_branch(prev_pointer, prev_length, pointer, 0);
          sizes[prev_index] -= sz;
          sizes[parent_index] += sz - 1;
          prev_pointer->length(prev_length);
          decrement_sizes(parent_pointer->parent_pointer,
                          parent_pointer->parent_index());

          return;
        }

        auto sz =
            move_range_branch(pointer, 0, prev_pointer, prev_length, index);
        sz += move_range_branch(pointer, index + 1, prev_pointer,
                                prev_length + index, length - index);
        prev_pointer->length(prev_length + length);
        sizes[prev_index] += sz;
        erase_index = parent_index;
      }

      else {
        auto next_index = parent_index + 1;
        auto next_pointer = static_traits::cast_node(pointers[next_index]);
        auto next_length = next_pointer->length();

        if (next_length != static_traits::base_min) {
          --next_length;
          move_backward_branch(pointer, length, index, 1);
          auto sz = move_single_branch(next_pointer, 0, pointer, length);
          move_backward_branch(next_pointer, next_length, 0, 1);
          sizes[next_index] -= sz;
          sizes[parent_index] += sz - 1;
          next_pointer->length(next_length);
          decrement_sizes(parent_pointer->parent_pointer,
                          parent_pointer->parent_index());

          return;
        }

        move_backward_branch(pointer, length, index, 1);
        auto sz =
            move_range_branch(next_pointer, 0, pointer, length, next_length);
        pointer->length(length + next_length);
        sizes[parent_index] += sz - 1;
        erase_index = next_index;
      }

      pointer = parent_pointer;
      index = erase_index;
    }
  }

  // helpers
  iterator_data find_index(size_type pos) const {
    return static_traits::find_index_root(get_root(), get_size(), get_height(),
                                          pos);
  }

  iterator_data find_first() const {
    return static_traits::find_first_root(get_root(), get_size(), get_height());
  }

  iterator_data find_last() const {
    return static_traits::find_last_root(get_root(), get_size(), get_height());
  }

  iterator_data find_end() const {
    return static_traits::find_end_root(get_root(), get_size(), get_height());
  }

  void steal(segmented_tree_seq &other) {
    get_root() = other.get_root();
    get_height() = other.get_height();
    get_size() = other.get_size();
    other.get_root() = nullptr;
    other.get_height() = 0;
    other.get_size() = 0;
  }

// debug functions
#ifdef BOOST_SEGMENTED_TREE_SEQ_DEBUG
  void verify_iterator(iterator_data a, size_type pos) {
    auto b = nth(pos).it_;
    if (a.entry.segment.pointer != b.entry.segment.pointer) {
      std::cerr << "segment pointer mismatch: " << a.entry.segment.pointer
                << " " << b.entry.segment.pointer << std::endl;
    }

    if (a.entry.segment.index != b.entry.segment.index) {
      std::cerr << "segment index mismatch: " << +a.entry.segment.index << " "
                << +b.entry.segment.index << std::endl;
    }

    if (a.entry.segment.length != b.entry.segment.length) {
      std::cerr << "segment length mismatch: " << +a.entry.segment.length << " "
                << +b.entry.segment.length << std::endl;
    }

    if (a.entry.leaf.pointer != b.entry.leaf.pointer) {
      std::cerr << "leaf pointer mismatch: " << a.entry.leaf.pointer << " "
                << b.entry.leaf.pointer << std::endl;
    }

    if (a.entry.leaf.index != b.entry.leaf.index) {
      std::cerr << "leaf index mismatch: " << +a.entry.leaf.index << " "
                << +b.entry.leaf.index << std::endl;
    }

    if (a.pos != b.pos) {
      std::cerr << "pos mismatch: " << +a.pos << " " << +b.pos << std::endl;
    }
  }
#else
  template <typename... Args>
  void verify_iterator(Args &&...) {}
#endif

 public:
  // public interface
  /// \brief Constructs an empty sequence using the specified allocator.
  ///
  /// \par Complexity
  ///   Constant.
  explicit segmented_tree_seq(Allocator const &alloc)
      : data_{nullptr, 0, 0, alloc, alloc} {}

  /// \brief Default constructs an empty sequence.
  ///
  /// \par Complexity
  ///   Constant.
  explicit segmented_tree_seq() noexcept(
      std::is_nothrow_default_constructible<allocator_type>::value)
      : segmented_tree_seq{Allocator()} {}

  /// \brief Constructs a count size sequence using the specified allocator,
  ///        each element copy constructed from value.
  ///
  /// \par Complexity
  ///   NlogN, where N is count.
  segmented_tree_seq(size_type count, T const &value,
                     Allocator const &alloc = Allocator())
      : segmented_tree_seq{alloc} {
    insert(end(), count, value);
  }

  /// \brief Constructs a count size sequence using the specified allocator,
  ///        each element default constructed.
  ///
  /// \par Complexity
  ///   NlogN, where N is count.
  explicit segmented_tree_seq(size_type count,
                              Allocator const &alloc = Allocator())
      : segmented_tree_seq{alloc} {
    insert(end(), count, value_type{});
  }

  /// \brief Constructs an empty sequence using the specified allocator, and
  ///        inserts elements from the range [first, last).
  ///
  /// \par Complexity
  ///   NlogN, where N is the size of the range.
  template <class InputIt,
            typename = typename std::iterator_traits<InputIt>::pointer>
  segmented_tree_seq(InputIt first, InputIt last,
                     Allocator const &alloc = Allocator())
      : segmented_tree_seq{alloc} {
    insert(end(), first, last);
  }

  /// \brief Copy constructs a sequence.
  ///
  /// \par Complexity
  ///   NlogN, where N is other.size().
  segmented_tree_seq(segmented_tree_seq const &other)
      : segmented_tree_seq{
            element_traits::select_on_container_copy_construction(
                other.get_element_allocator())} {
    insert(end(), other.begin(), other.end());
  }

  /// \brief Copy constructs a sequence using the specified allocator.
  ///
  /// \par Complexity
  ///   NlogN, where N is other.size().
  segmented_tree_seq(segmented_tree_seq const &other, Allocator const &alloc)
      : segmented_tree_seq{alloc} {
    insert(end(), other.begin(), other.end());
  }

  /// \brief Move constructs a sequence.
  ///
  /// \par Complexity
  ///   Constant.
  segmented_tree_seq(segmented_tree_seq &&other) noexcept(
      std::is_nothrow_move_constructible<allocator_type>::value)
      : segmented_tree_seq{std::move(other.get_element_allocator())} {
    steal(other);
  }

  /// \brief Move constructs a sequence using the specified allocator.
  ///
  /// \par Complexity
  ///   Constant if alloc compares equal to other's allocator. NlogN, where N is
  ///   other.size() otherwise.
  segmented_tree_seq(segmented_tree_seq &&other, Allocator const &alloc)
      : segmented_tree_seq{alloc} {
    if (get_element_allocator() == other.get_element_allocator())
      steal(other);
    else
      insert(end(), std::make_move_iterator(other.begin()),
             std::make_move_iterator(other.end()));
  }

  /// \brief Constructs an empty sequence using the specified allocator, and
  ///        inserts elements from init.
  ///
  /// \par Complexity
  ///   NlogN, where N is init.size().
  segmented_tree_seq(std::initializer_list<T> init,
                     Allocator const &alloc = Allocator())
      : segmented_tree_seq{alloc} {
    insert(end(), init.begin(), init.end());
  }

  /// \brief Destructs the sequence releasing all memory.
  ///
  /// \par Complexity
  ///   Linear in size().
  ~segmented_tree_seq() { purge(); }

  /// \brief Copy assigns a sequence.
  ///
  /// \par Complexity
  ///   Linear in the minimum of size() and other.size(), plus MlogN, where M is
  ///   the difference of size() and other.size(), and N is the maximum of
  ///   size() and other.size().
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   Basic.
  segmented_tree_seq &operator=(segmented_tree_seq const &other) {
    if (this != &other) {
      if (element_traits::propagate_on_container_copy_assignment::value) {
        if (get_element_allocator() != other.get_element_allocator()) clear();
        get_element_allocator() = other.get_element_allocator();
        get_node_allocator() = other.get_node_allocator();
      }
      assign(other.begin(), other.end());
    }
    return *this;
  }

  /// \brief Move assigns a sequence.
  ///
  /// \par Complexity
  ///   Linear in size() if the allocator propagates on move assignment or the
  ///   sequence's allocator compares equal to other's allocator. Linear in the
  ///   minimum of size() and other.size(), plus MlogN, where M is the
  ///   difference of size() and other.size(), and N is the maximum of size()
  ///   and other.size() otherwise.
  ///
  /// \par Iterator invalidation
  ///   No iterators are invalidated if the allocator propagates on move
  ///   assignment or the sequence's allocator compares equal to other's
  ///   allocator. Invalidates all iterators otherwise.
  ///
  /// \par Exception safety
  ///   No-throw if the allocator propagates on move assignment or the
  ///   sequence's allocator compares equal to other's allocator. Basic
  ///   otherwise.
  segmented_tree_seq &operator=(segmented_tree_seq &&other) noexcept(
      allocator_type::propagate_on_container_move_assignment::value
          &&std::is_nothrow_move_assignable<allocator_type>::value) {
    if (element_traits::propagate_on_container_move_assignment::value ||
        get_element_allocator() == other.get_element_allocator()) {
      purge();
      if (element_traits::propagate_on_container_move_assignment::value) {
        get_element_allocator() = other.get_element_allocator();
        get_node_allocator() = other.get_node_allocator();
      }
      steal(other);
    } else
      assign(std::make_move_iterator(other.begin()),
             std::make_move_iterator(other.end()));

    return *this;
  }

  /// \brief Assigns the sequence to count elements copy constructed from value.
  ///
  /// \par Complexity
  ///   Linear in the minimum of size() and ilist.size(), plus MlogN, where M is
  ///   the difference of size() and ilist.size(), and N is the maximum of
  ///   size() and ilist.size().
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   Basic.
  segmented_tree_seq &operator=(std::initializer_list<T> ilist) {
    assign(ilist.begin(), ilist.end());
    return *this;
  }

  /// \brief Assigns the sequence to count elements copy constructed from
  ///        value.
  ///
  /// \par Complexity
  ///   Linear in the minimum of size() and count, plus MlogN, where M is the
  ///   difference of size() and count, and N is the maximum of size() and
  ///   count.
  ///
  /// \par Iterator invalidation
  ///  Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   Basic.
  void assign(size_type count, T const &value) {
    auto first = begin();
    auto last = end();
    while (true) {
      if (count == 0) {
        erase(first, last);
        return;
      }

      if (first == last) {
        insert(last, count, value);
        return;
      }

      *first = value;
      ++first;
      --count;
    }
  }

  /// \brief Assigns the sequence to the elements copy constructed from the
  ///        range [first, last).
  ///
  /// \par Complexity
  ///   Linear in the minimum of size() and the size of the range, plus MlogN,
  ///   where M is the difference of size() and the size of the range, and N is
  ///   the maximum of size() and the size of the range.
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   Basic.
  template <class InputIt,
            typename = typename std::iterator_traits<InputIt>::pointer>
  void assign(InputIt source_first, InputIt source_last) {
    auto first = begin();
    auto last = end();
    while (true) {
      if (source_first == source_last) {
        erase(first, last);
        return;
      }

      if (first == last) {
        insert(last, source_first, source_last);
        return;
      }

      *first = *source_first;
      ++first;
      ++source_first;
    }
  }

  /// \brief Assigns the sequence to the elements of the specified
  ///        initializer_list.
  ///
  /// \par Complexity
  ///   Linear in the minimum of size() and ilist.size(), plus MlogN, where M is
  ///   the difference of size() and ilist.size(), and N is the maximum of
  ///   size() and ilist.size().
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   Basic.
  void assign(std::initializer_list<T> ilist) {
    assign(ilist.begin(), ilist.end());
  }

  /// \brief Returns a copy of the allocator for the sequence.
  ///
  /// \par Complexity
  ///   Constant.
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   No-throw.
  allocator_type get_allocator() const noexcept {
    return get_element_allocator();
  }

  /// \brief Returns a reference for the object located at the index pos.
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   Strong.
  reference at(size_type pos) {
    if (pos > size())
      throw std::out_of_range{"segmented_tree_seq at() out of bounds"};
    return (*this)[pos];
  }

  /// \brief Returns a const_reference for the object located at the specified
  ///        index pos.
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   Strong.
  const_reference at(size_type pos) const {
    if (pos > size())
      throw std::out_of_range{"segmented_tree_seq at() out of bounds"};
    return (*this)[pos];
  }

  /// \brief Returns a reference for the object located at the specified index
  ///        pos.
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   Strong.
  reference operator[](size_type pos) {
    iterator it = find_index(pos);
    return *it;
  }

  /// \brief Returns a const_reference for the object located at the specified
  ///        index pos.
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   Strong.
  const_reference operator[](size_type pos) const {
    iterator it = find_index(pos);
    return *it;
  }

  /// \brief Returns a reference for the object located at the index 0.
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   Strong.
  reference front() { return *begin(); }

  /// \brief Returns a const_reference for the object located at the index 0.
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   Strong.
  const_reference front() const { return *begin(); }

  /// \brief Returns a reference for the object located at the index size() - 1.
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   Strong.
  reference back() { return *penultimate(); }

  /// \brief Returns a const_reference for the object located at the index
  ///        size() - 1.
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   Strong.
  const_reference back() const { return *penultimate(); }

  /// \brief Returns an iterator for the index 0.
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   No-throw.
  iterator begin() noexcept { return find_first(); }

  /// \brief Returns a const_iterator for the index 0.
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   No-throw.
  const_iterator begin() const noexcept { return find_first(); }

  /// \brief Returns a const_iterator for the index 0.
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   No-throw.
  const_iterator cbegin() const noexcept { return find_first(); }

  /// \brief Returns an iterator to the index size() - 1
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   No-throw.
  ///
  /// \note
  ///   Non-standard extension.
  iterator penultimate() noexcept { return find_last(); }

  /// \brief Returns a const_iterator to the index size() - 1
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   No-throw.
  ///
  /// \note
  ///   Non-standard extension.
  const_iterator penultimate() const noexcept { return find_last(); }

  /// \brief Returns a const_iterator to the index size() - 1
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   No-throw.
  ///
  /// \note
  ///   Non-standard extension.
  const_iterator cpenultimate() const noexcept { return find_last(); }

  /// \brief Returns an iterator for the index size().
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   No-throw.
  iterator end() noexcept { return find_end(); }

  /// \brief Returns a const_iterator for the index size().
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   No-throw.
  const_iterator end() const noexcept { return find_end(); }

  /// \brief Returns a const_iterator for the index size().
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   No-throw.
  const_iterator cend() const noexcept { return find_end(); }

  /// \brief Returns a reverse_iterator for the index size().
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   No-throw.
  reverse_iterator rbegin() noexcept { return reverse_iterator{end()}; }

  /// \brief Returns a const_reverse_iterator for the index size().
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   No-throw.
  const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator{end()};
  }

  /// \brief Returns a const_reverse_iterator for the index size().
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   No-throw.
  const_reverse_iterator crbegin() const noexcept {
    return const_reverse_iterator{end()};
  }

  /// \brief Returns a reverse_iterator for the index 0.
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   No-throw.
  reverse_iterator rend() noexcept { return reverse_iterator{begin()}; }

  /// \brief Returns a const_reverse_iterator for the index 0.
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   No-throw.
  const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator{begin()};
  }

  /// \brief Returns a const_reverse_iterator for the index 0.
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   No-throw.
  const_reverse_iterator crend() const noexcept {
    return const_reverse_iterator{begin()};
  }

  /// \brief Returns an iterator for the index pos.
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   No-throw.
  ///
  /// \note
  ///   Non-standard extension.
  ///
  /// \note
  ///   This member function is more efficient than calling begin() and
  ///   advancing.
  iterator nth(size_type pos) noexcept {
    if (pos >= size()) return find_end();
    return find_index(pos);
  }

  /// \brief Returns a const_iterator for the index pos.
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   No-throw.
  ///
  /// \note
  ///   Non-standard extension.
  ///
  /// \note
  ///   This member function is more efficient than calling begin() and
  ///   advancing.
  const_iterator nth(size_type pos) const noexcept {
    if (pos >= size()) return find_end();
    return find_index(pos);
  }

  /// \brief Returns the index of the specified iterator.
  ///
  /// \par Complexity
  ///   Constant.
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   No-throw.
  ///
  /// \note
  ///   Non-standard extension.
  size_type index_of(iterator pos) noexcept { return pos.it_.pos; }

  /// \brief Returns the index of the specified const_iterator.
  ///
  /// \par Complexity
  ///   Constant.
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   No-throw.
  ///
  /// \note
  ///   Non-standard extension.
  size_type index_of(const_iterator pos) const noexcept { return pos.it_.pos; }

  /// \brief Returns true if the sequence is empty, false otherwise.
  ///
  /// \par Complexity
  ///   Constant.
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   No-throw.
  bool empty() const noexcept { return get_size() == 0; }

  /// \brief Returns the count of elements stored in the sequence.
  ///
  /// \par Complexity
  ///   Constant.
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   No-throw.
  size_type size() const noexcept { return get_size(); }

  /// \brief Returns the height of the tree.
  ///
  /// \par Complexity
  ///   Constant.
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   No-throw.
  ///
  /// \note
  ///   Non-standard extension.
  size_type height() const noexcept { return get_height(); }

  /// \brief Returns the maximum count of elements able to be stored in the
  ///        sequence.
  ///
  /// \par Complexity
  ///   Constant.
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated.
  ///
  /// \par Exception safety
  ///   No-throw.
  size_type max_size() const noexcept {
    return (std::numeric_limits<size_type>::max)();
  }

  /// \brief Removes all elements from the sequence.
  ///
  /// \par Complexity
  ///   Linear in size().
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   No-throw.
  void clear() noexcept {
    purge();
    get_root() = nullptr;
    get_height() = 0;
    get_size() = 0;
  }

  /// \brief Copy constructs an object at the specified position.
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   Strong.
  iterator insert(const_iterator pos, T const &value) {
    return emplace(pos, value);
  }

  /// \brief Move construct an object at the specified position.
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   Strong.
  iterator insert(const_iterator pos, T &&value) {
    return emplace(pos, std::move(value));
  }

  /// \brief Copy construct count elements at the specified position.
  ///
  /// \par Complexity
  ///   MlogN, where M is count, and N is the maximum of size() and count.
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   Basic.
  iterator insert(const_iterator pos, size_type count, T const &value) {
    for (size_type i = 0; i != count; ++i) {
      pos = insert(pos, value);
      ++pos;
    }
    static_traits::move_prev_segment_count(pos.it_.entry, count);
    return pos.it_;
  }

  /// \brief Copy constructs all elements in the range [first, last) at the
  ///        specified position.
  ///
  /// \par Complexity
  ///   MlogN, where M is the size of the range, and N is the maximum of size()
  ///   and the size of the range.
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   Basic.
  template <class InputIt,
            typename = typename std::iterator_traits<InputIt>::pointer>
  iterator insert(const_iterator pos, InputIt first, InputIt last) {
    size_type count = 0;
    while (first != last) {
      pos = insert(pos, *first);
      ++first;
      ++pos;
      ++count;
    }
    static_traits::move_prev_segment_count(pos.it_.entry, count);
    return pos.it_;
  }

  /// \brief Copy constructs all elements in the specified initializer_list at
  ///        the specified position.
  ///
  /// \par Complexity
  ///   MlogN, where M is ilist.size(), and N is the maximum of size() and
  ///   ilist.size().
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   Basic.
  iterator insert(const_iterator pos, std::initializer_list<T> ilist) {
    return insert(pos, ilist.begin(), ilist.end());
  }

  /// \brief Forward constructs an object at the specified position.
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   Strong.
  template <class... Args>
  iterator emplace(const_iterator pos, Args &&... args) {
    auto it = reserve_single_iterator(pos.it_);
    try {
      copy_single_segment(it.entry.segment.pointer, it.entry.segment.index,
                          std::forward<Args>(args)...);
    } catch (...) {
      erase_single_segment(it.entry);
      throw;
    }

    verify_iterator(it, pos.it_.pos);
    return it;
  }

  /// \brief Remove the object at the specified position from the sequence.
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   Strong.
  iterator erase(const_iterator pos) {
    auto it = erase_single_iterator(pos.it_);
    verify_iterator(it, pos.it_.pos);
    return it;
  }

  /// \brief Remove all elements in the range [first, last) from the sequence.
  ///
  /// \par Complexity
  ///   MlogN, where M is the size of the range, and N is size().
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   Basic.
  iterator erase(const_iterator first, const_iterator last) {
    while (first != last) {
      --last;
      last = erase(last);
    }
    return last.it_;
  }

  /// \brief Copy constructs an object at end().
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   Strong.
  void push_back(T const &value) { emplace_back(value); }

  /// \brief Move constructs an object at end().
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   Strong.
  void push_back(T &&value) { emplace_back(std::move(value)); }

  /// \brief Forward constructs an object at end().
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   Strong.
  template <class... Args>
  void emplace_back(Args &&... args) {
    emplace(end(), std::forward<Args>(args)...);
  }

  /// \brief Removes the object at end() - 1.
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   Strong.
  void pop_back() { erase(penultimate()); }

  /// \brief Copy constructs an object at begin().
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   Strong.
  void push_front(const T &value) { emplace_front(value); }

  /// \brief Move constructs an object at begin().
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   Strong.
  void push_front(T &&value) { emplace_front(std::move(value)); }

  /// \brief Forward constructs an object at begin().
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   Strong.
  template <class... Args>
  void emplace_front(Args &&... args) {
    emplace(begin(), std::forward<Args>(args)...);
  }

  /// \brief Removes the object at begin().
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   Strong.
  void pop_front() { erase(begin()); }

  /// \brief Resizes the seqeuence to the specified size, default construct any
  ///        elements above the current size.
  ///
  /// \par Complexity
  ///   MlogN, where M is the difference of size() and count, and N is the max
  ///   of size() and count.
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   Strong.
  void resize(size_type count) { resize(count, {}); }

  /// \brief Resizes the seqeuence to the specified size, copy construct any
  ///        elements from value above the current size.
  ///
  /// \par Complexity
  ///   MlogN, where M is the difference of size() and count, and N is the max
  ///   of size() and count.
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   Basic.
  void resize(size_type count, value_type const &value) {
    auto sz = size();
    if (sz == count) return;

    auto last = end();
    if (count < sz)
      erase(nth(count), last);
    else
      insert(last, count, value);
  }

  /// \brief Swaps the contents *this with the specified sequence.
  ///
  /// \par Complexity
  ///   Constant.
  ///
  /// \par Iterator invalidation
  ///   Every iterator referring to an element in one container before the
  ///   swap shall refer to the same element in the other container after the
  ///   swap.
  ///
  /// \par Exception safety
  ///   No-throw if the allocator propagates on swap or the allocator doesn't
  ///   throw on swap. Strong otherwise.
  void swap(segmented_tree_seq &other) noexcept(
      !element_traits::propagate_on_container_swap::value ||
      segmented_tree_seq_detail::is_nothrow_swappable<allocator_type>::value) {
    using std::swap;
    if (element_traits::propagate_on_container_swap::value) {
      swap(get_element_allocator(), other.get_element_allocator());
      swap(get_node_allocator(), other.get_node_allocator());
    }
    swap(get_root(), other.get_root());
    swap(get_height(), other.get_height());
    swap(get_size(), other.get_size());
  }

  //  TODO
  //  void merge(segmented_tree_seq &other);
  //  void merge(segmented_tree_seq &&other);
  //  template <class Compare>
  //  void merge(segmented_tree_seq &other, Compare comp);
  //  template <class Compare>
  //  void merge(segmented_tree_seq &&other, Compare comp);

  //  void splice(const_iterator pos, segmented_tree_seq &other);
  //  void splice(const_iterator pos, segmented_tree_seq &&other);
  //  void splice(const_iterator pos, segmented_tree_seq &other, const_iterator
  //  it);
  //  void splice(const_iterator pos, segmented_tree_seq &&other, const_iterator
  //  it);
  //  void splice(const_iterator pos, segmented_tree_seq &other, const_iterator
  //  first,
  //              const_iterator last);
  //  void splice(const_iterator pos, segmented_tree_seq &&other, const_iterator
  //  first,
  //              const_iterator last);

  /// \brief Removes all elements matching the specified value.
  ///
  /// \par Complexity
  ///   Logarithmic in size().
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   Basic.
  void remove(const T &value) {
    erase(std::remove(begin(), end(), value), end());
  }

  /// \brief Removes all elements matching the specified predicate.
  ///
  /// \par Complexity
  ///   Linear in size().
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   Basic.
  template <class UnaryPredicate>
  void remove_if(UnaryPredicate p) {
    erase(std::remove_if(begin(), end(), p), end());
  }

  /// \brief Reverses the sequence.
  ///
  /// \par Complexity
  ///   Linear in size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated, but reflect the change in ordering.
  ///
  /// \par Exception safety
  ///   Basic.
  void reverse() { std::reverse(begin(), end()); }

  /// \brief Removes all consecutive duplicate elements from the sequence.
  ///
  /// \par Complexity
  ///   Linear in size().
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   Basic.
  void unique() { unique(std::equal_to<value_type>{}); }

  /// \brief Removes all consecutive duplicate elements from the
  ///        sequence using the specified predicate.
  ///
  /// \par Complexity
  ///   Linear in size().
  ///
  /// \par Iterator invalidation
  ///   Invalidates all iterators.
  ///
  /// \par Exception safety
  ///   Basic.
  template <class BinaryPredicate>
  void unique(BinaryPredicate p) {
    erase(std::unique(begin(), end(), p), end());
  }

  /// \brief Stable sorts the sequence.
  ///
  /// \par Complexity
  ///   NlogN, where N is size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated, but reflect the change in ordering.
  ///
  /// \par Exception safety
  ///   Basic.
  void sort() { sort(std::less<value_type>{}); }

  /// \brief Stable sort the sequence using the specified predicate.
  ///
  /// \par Complexity
  ///    NlogN, where N is size().
  ///
  /// \par Iterator invalidation
  ///   Iterators are not invalidated, but reflect the change in ordering.
  ///
  /// \par Exception safety
  ///   Basic.
  template <class Compare>
  void sort(Compare comp) {
    std::stable_sort(begin(), end(), comp);
  }
};

// free functions

/// \brief Returns true if both sequences are of the same length and have each
///        element in both sequences are equal. Returns false otherwise.
///
/// \par Complexity
///    Linear in size().
///
/// \par Iterator invalidation
///   Iterators are not invalidated.
///
/// \par Exception safety
///   Strong.
template <typename T, typename Alloc, size_t... Args>
bool operator==(segmented_tree_seq<T, Alloc, Args...> &lhs,
                segmented_tree_seq<T, Alloc, Args...> &rhs) {
  return lhs.size() == rhs.size() &&
         std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

/// \brief Returns false if both sequences are of the same length and have each
///        element in both sequences are equal. Returns true otherwise.
///
/// \par Complexity
///   Linear in size().
///
/// \par Iterator invalidation
///   Iterators are not invalidated.
///
/// \par Exception safety
///   Strong.
template <typename T, typename Alloc, size_t... Args>
bool operator!=(segmented_tree_seq<T, Alloc, Args...> &lhs,
                segmented_tree_seq<T, Alloc, Args...> &rhs) {
  return !(lhs == rhs);
}

/// \brief Returns true if the first sequence is lexicographically less than the
///        second. Returns false otherwise.
///
/// \par Complexity
///   Linear in size().
///
/// \par Iterator invalidation
///   Iterators are not invalidated.
///
/// \par Exception safety
///   Strong.
template <typename T, typename Alloc, size_t... Args>
bool operator<(segmented_tree_seq<T, Alloc, Args...> &lhs,
               segmented_tree_seq<T, Alloc, Args...> &rhs) {
  return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(),
                                      rhs.end());
}

/// \brief Returns true if the first sequence is equal or lexicographically less
///        than the second. Returns false otherwise.
///
/// \par Complexity
///   Linear in size().
///
/// \par Iterator invalidation
///   Iterators are not invalidated.
///
/// \par Exception safety
///   Strong.
template <typename T, typename Alloc, size_t... Args>
bool operator<=(segmented_tree_seq<T, Alloc, Args...> &lhs,
                segmented_tree_seq<T, Alloc, Args...> &rhs) {
  return !(rhs < lhs);
}

/// \brief Returns true if the first sequence is lexicographically greater than
///        the second. Returns false otherwise.
///
/// \par Complexity
///   Linear in size().
///
/// \par Iterator invalidation
///   Iterators are not invalidated.
///
/// \par Exception safety
///   Strong.
template <typename T, typename Alloc, size_t... Args>
bool operator>(segmented_tree_seq<T, Alloc, Args...> &lhs,
               segmented_tree_seq<T, Alloc, Args...> &rhs) {
  return rhs < lhs;
}

/// \brief Returns true if the first sequence is lexicographically greater or
///        equal to the second. Returns false otherwise.
///
/// \par Complexity
///   Linear in size().
///
/// \par Iterator invalidation
///   Iterators are not invalidated.
///
/// \par Exception safety
///   Strong.
template <typename T, typename Alloc, size_t... Args>
bool operator>=(segmented_tree_seq<T, Alloc> &lhs,
                segmented_tree_seq<T, Alloc> &rhs) {
  return !(lhs < rhs);
}

/// \brief Swaps the contents of the sequences.
///
/// \par Complexity
///   Constant.
///
/// \par Iterator invalidation
///   Every iterator referring to an element in one container before the
///   swap shall refer to the same element in the other container after the
///   swap.
///
/// \par Exception safety
///   No-throw if the allocator propagates on swap or the allocator doesn't
///   throw on swap. Strong otherwise.
template <typename T, typename Traits, typename Alloc, size_t... Args>
void swap(
    segmented_tree_seq<T, Alloc, Args...> &a,
    segmented_tree_seq<T, Alloc, Args...> &b) noexcept(noexcept(a.swap(b))) {
  a.swap(b);
}
}
}

#endif  // #ifndef BOOST_CONTAINER_SEGMENTED_TREE_SEQ
