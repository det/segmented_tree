#pragma once

#include <array>
#include <algorithm>
#include <cassert>
#include <cstring>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <vector>
#include <type_traits>

namespace boost {
namespace container {

template <typename T, typename Allocator = std::allocator<T>,
          std::size_t segment_target = 512, std::size_t base_target = 512>
class segmented_tree_seq {
  // forward declarations
 private:
  struct segment_entry;
  struct leaf_entry;
  struct iterator_entry;
  struct iterator_data;
  struct base_node;
  struct leaf_data;
  struct branch_data;
  struct leaf_node;
  struct branch_node;
  union node;
  template <typename Reference, typename Pointer>
  class iterator_t;

  using element_traits = typename std::allocator_traits<Allocator>;
  using element_pointer = typename element_traits::pointer;
  using node_allocator = typename element_traits::template rebind_alloc<node>;
  using node_traits = typename element_traits::template rebind_traits<node>;
  using node_pointer = typename node_traits::pointer;
  using void_pointer = typename element_traits::void_pointer;

 public:
  using value_type = typename element_traits::value_type;
  using allocator_type = Allocator;
  using size_type = typename element_traits::size_type;
  using difference_type = typename element_traits::difference_type;
  using reference = value_type &;
  using const_reference = value_type const &;
  using pointer = typename element_traits::pointer;
  using const_pointer = typename element_traits::const_pointer;
  using iterator = iterator_t<reference, value_type *>;
  using const_iterator = iterator_t<const_reference, value_type const *>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

 private:
  // private types
  struct base_node {
    branch_node *parent_pointer;
    std::uint16_t parent_index;
    std::uint16_t length;
  };

  struct leaf_data {
    element_pointer pointer;
    size_type size;
  };

  struct branch_data {
    node_pointer pointer;
    size_type size;
  };

  // private constexpr
  static constexpr bool is_trivial_ = std::is_trivial<T>::value;
  static constexpr size_type segment_free = segment_target;
  static constexpr size_type base_free = base_target - sizeof(base_node);
  static constexpr size_type segment_fit = segment_free / sizeof(T);
  static constexpr size_type base_fit = base_free / sizeof(branch_data);
  static constexpr size_type segment_max = segment_fit > 1 ? segment_fit : 1;
  static constexpr size_type base_max = base_fit > 3 ? base_fit : 3;
  static constexpr size_type segment_min = (segment_max + 1) / 2;
  static constexpr size_type base_min = (base_max + 1) / 2;

  // private types
  struct leaf_node {
    branch_node *parent_pointer;
    std::uint16_t parent_index;
    std::uint16_t length;
    std::array<size_type, base_max> sizes;
    std::array<element_pointer, base_max> pointers;
  };

  struct branch_node {
    branch_node *parent_pointer;
    std::uint16_t parent_index;
    std::uint16_t length;
    std::array<size_type, base_max> sizes;
    std::array<node_pointer, base_max> pointers;
  };

  union node {
    base_node base;
    leaf_node leaf;
    branch_node branch;
  };

  struct segment_entry {
    element_pointer pointer;
    size_type index;
    size_type length;
  };

  struct leaf_entry {
    leaf_node *pointer;
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

  template <typename Reference, typename Pointer>
  class iterator_t {
    friend segmented_tree_seq;

   private:
    iterator_data it_;
    iterator_t(iterator_data it) : it_(it) {}

   public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using reference = Reference;
    using pointer = Pointer;

    iterator_t() = default;
    iterator_t(iterator const &other) : it_(other.it_) {}

    std::size_t index() const { return it_.entry.segment.index; }
    pointer begin() const { return std::addressof(*it_.entry.segment.pointer); }
    pointer end() const { return begin() + it_.entry.segment.length; }
    pointer operator->() const { return begin() + index(); }
    reference operator*() const { return begin()[index()]; }

    iterator_t &operator++() {
      move_next_iterator(it_);
      return *this;
    }

    iterator_t &operator--() {
      move_prev_iterator(it_);
      return *this;
    }

    iterator_t operator++(int) {
      iterator_data copy = it_;
      move_next_iterator(it_);
      return copy;
    }

    iterator_t operator--(int) {
      iterator_data copy = it_;
      move_prev_iterator(it_);
      return copy;
    }

    iterator_t &operator+=(difference_type diff) {
      move_iterator_count(it_, diff);
      return *this;
    }

    iterator_t &operator-=(difference_type diff) {
      move_iterator_count(it_, -diff);
      return *this;
    }

    iterator_t operator+(difference_type diff) const {
      auto copy = it_;
      move_iterator_count(copy, diff);
      return copy;
    }

    iterator_t operator-(difference_type diff) const {
      auto copy = it_;
      move_iterator_count(copy, -diff);
      return copy;
    }

    difference_type operator-(iterator_t const &other) const {
      return it_.pos - other.it_.pos;
    }

    iterator_t &move_before_segment() {
      move_before_segment(it_);
      return *this;
    }

    iterator_t &move_before_segment(size_type count) {
      move_before_segment_count(it_, count);
      return *this;
    }

    iterator_t &move_after_segment() {
      move_after_segment(it_);
      return *this;
    }

    iterator_t &move_after_segment(size_type count) {
      move_after_segment_count(it_, count);
      return *this;
    }

    iterator_t before_segment() const {
      auto copy = it_;
      move_before_segment(copy);
      return copy;
    }

    iterator_t after_segment() const {
      auto copy = it_;
      move_after_segment(copy);
      return copy;
    }

    iterator_t before_segment(size_type count) const {
      auto copy = it_;
      move_before_segment_count(copy, count);
      return copy;
    }

    iterator_t after_segment(size_type count) const {
      auto copy = it_;
      move_after_segment_count(copy, count);
      return copy;
    }

    bool operator!=(iterator_t const &other) const {
      return it_.pos != other.it_.pos;
    }
    bool operator==(iterator_t const &other) const {
      return it_.pos == other.it_.pos;
    }
    bool operator<(iterator_t const &other) const {
      return it_.pos < other.it_.pos;
    }
    bool operator>(iterator_t const &other) const {
      return it_.pos > other.it_.pos;
    }
    bool operator<=(iterator_t const &other) const {
      return it_.pos <= other.it_.pos;
    }
    bool operator>=(iterator_t const &other) const {
      return it_.pos >= other.it_.pos;
    }
  };

  // cast
  static element_pointer cast_segment(void_pointer pointer) {
    return static_cast<element_pointer>(pointer);
  }

  static node_pointer cast_node(void_pointer pointer) {
    return static_cast<node_pointer>(pointer);
  }

  // find_index
  static iterator_data find_index_root(void_pointer pointer, size_type size,
                                       size_type height, size_type pos) {
    iterator_data it;
    it.pos = pos;

    if (height == 0) {
      it.entry.leaf.pointer = nullptr;
      it.entry.leaf.index = 0;
      it.entry.segment = find_index_segment(cast_segment(pointer), size, pos);
    } else
      it.entry = find_index_node(cast_node(pointer), height, pos);

    return it;
  }

  static iterator_entry find_index_node(node_pointer pointer, size_type height,
                                        size_type pos) {
    if (height == 1) return find_index_leaf(&pointer->leaf, pos);
    return find_index_branch(&pointer->branch, height, pos);
  }

  static iterator_entry find_index_branch(branch_node *pointer,
                                          size_type height, size_type pos) {
    while (true) {
      size_type index = 0;
      while (true) {
        auto size = pointer->sizes[index];
        if (pos < size) break;
        pos -= size;
        ++index;
      }

      auto child = pointer->pointers[index];
      --height;
      if (height == 1) return find_index_leaf(&child->leaf, pos);

      pointer = &child->branch;
    }
  }

  static iterator_entry find_index_leaf(leaf_node *pointer, size_type pos) {
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
    entry.segment = find_index_segment(pointer->pointers[index],
                                       pointer->sizes[index], pos);
    return entry;
  }

  static segment_entry find_index_segment(element_pointer pointer,
                                          size_type size, size_type pos) {
    segment_entry entry;
    entry.pointer = pointer;
    entry.index = pos;
    entry.length = size;
    return entry;
  }

  // find_first
  static iterator_data find_first_root(void_pointer pointer, size_type size,
                                       size_type height) {
    iterator_data it;
    it.pos = 0;

    if (height == 0) {
      it.entry.leaf.pointer = nullptr;
      it.entry.leaf.index = 0;
      it.entry.segment = find_first_segment(cast_segment(pointer), size);
    } else
      it.entry = find_first_node(cast_node(pointer), height);

    return it;
  }

  static iterator_entry find_first_node(node_pointer pointer,
                                        size_type height) {
    if (height == 1) return find_first_leaf(&pointer->leaf);
    return find_first_branch(&pointer->branch, height);
  }

  static iterator_entry find_first_branch(branch_node *pointer,
                                          size_type height) {
    while (true) {
      auto child = pointer->pointers[0];
      --height;
      if (height == 1) return find_first_leaf(&child->leaf);

      pointer = &child->branch;
    }
  }

  static iterator_entry find_first_leaf(leaf_node *pointer) {
    iterator_entry entry;
    entry.leaf.pointer = pointer;
    entry.leaf.index = 0;
    entry.segment = find_first_segment(pointer->pointers[0], pointer->sizes[0]);
    return entry;
  }

  static segment_entry find_first_segment(element_pointer pointer,
                                          size_type size) {
    segment_entry entry;
    entry.pointer = pointer;
    entry.index = 0;
    entry.length = size;
    return entry;
  }

  // find_last
  static iterator_data find_last_root(void_pointer pointer, size_type size,
                                      size_type height) {
    iterator_data it;
    it.pos = size - 1;

    if (height == 0) {
      it.leaf.pointer = nullptr;
      it.leaf.index = 0;
      it.leaf.segment = find_last_segment(cast_segment(pointer), size);
    } else
      it.leaf = find_last_node(cast_node(pointer), height);

    return it;
  }

  static iterator_entry find_last_node(node_pointer pointer, size_type height) {
    if (height == 1) return find_last_leaf(&pointer->leaf);
    return find_last_branch(&pointer->branch, height);
  }

  static iterator_entry find_last_branch(branch_node *pointer,
                                         size_type height) {
    while (true) {
      size_type index = pointer->length - 1;
      auto child = pointer->pointers[index];
      --height;
      if (height == 1) return find_last_leaf(&child->leaf);

      pointer = &child->branch;
    }
  }

  static iterator_entry find_last_leaf(leaf_node *pointer) {
    iterator_entry entry;
    entry.leaf.pointer = pointer;
    entry.leaf.index = pointer->length - 1;
    entry.segment = find_last_segment(pointer->pointers[entry.leaf.index],
                                      pointer->sizes[entry.leaf.index]);
    return entry;
  }

  static segment_entry find_last_segment(element_pointer pointer,
                                         size_type size) {
    segment_entry entry;
    entry.pointer = pointer;
    entry.index = size - 1;
    entry.length = size;
    return entry;
  }

  // find_end
  static iterator_data find_end_root(void_pointer pointer, size_type size,
                                     size_type height) {
    iterator_data it;
    it.pos = size;

    if (height == 0) {
      it.entry.leaf.pointer = nullptr;
      it.entry.leaf.index = 0;
      it.entry.segment = find_end_segment(cast_segment(pointer), size);
    } else
      it.entry = find_end_node(cast_node(pointer), height);

    return it;
  }

  static iterator_entry find_end_node(node_pointer pointer, size_type height) {
    if (height == 1) return find_end_leaf(&pointer->leaf);
    return find_end_branch(&pointer->branch, height);
  }

  static iterator_entry find_end_branch(branch_node *pointer,
                                        size_type height) {
    while (true) {
      size_type index = pointer->length - 1;
      auto child = pointer->pointers[index];

      --height;
      if (height == 1) return find_end_leaf(&child->leaf);

      pointer = &child->branch;
    }
  }

  static iterator_entry find_end_leaf(leaf_node *pointer) {
    iterator_entry entry;
    entry.leaf.pointer = pointer;
    entry.leaf.index = pointer->length - 1;
    entry.segment = find_end_segment(pointer->pointers[entry.leaf.index],
                                     pointer->sizes[entry.leaf.index]);
    return entry;
  }

  static segment_entry find_end_segment(element_pointer pointer,
                                        size_type size) {
    segment_entry entry;
    entry.pointer = pointer;
    entry.index = size;
    entry.length = size;
    return entry;
  }

  // move_next
  static void move_next_iterator(iterator_data &it) {
    ++it.pos;
    move_next_segment(it.entry, it.entry.segment.index,
                      it.entry.segment.length);
  }

  static void move_next_segment(iterator_entry &entry, size_type index,
                                size_type length) {
    ++index;
    if (index != length) {
      entry.segment.index = index;
      return;
    }

    move_next_leaf(entry, entry.leaf.pointer, entry.leaf.index);
  }

  static void move_next_leaf(iterator_entry &entry, leaf_node *pointer,
                             size_type index) {
    // Special case for end iterator.
    if (pointer == nullptr) {
      entry.segment.index = entry.segment.length;
      return;
    }

    ++index;
    if (index != pointer->length) {
      entry.leaf.index = index;
      entry.segment =
          find_first_segment(pointer->pointers[index], pointer->sizes[index]);
      return;
    }

    move_next_branch(entry, pointer->parent_pointer, pointer->parent_index);
  }

  static void move_next_branch(iterator_entry &entry, branch_node *pointer,
                               size_type index) {
    size_type height = 2;

    while (true) {
      // Special case for end iterator.
      if (pointer == nullptr) {
        entry.segment.index = entry.segment.length;
        return;
      }

      ++index;
      if (index != pointer->length) {
        entry = find_first_node(pointer->pointers[index], height - 1);
        return;
      }

      index = pointer->parent_index;
      pointer = pointer->parent_pointer;
      ++height;
    }
  }

  // move_prev
  static void move_prev_iterator(iterator_data &it) {
    --it.pos;
    move_prev_segment(it.entry, it.entry.segment.index);
  }

  static void move_prev_segment(iterator_entry &entry, size_type index) {
    if (index != 0) {
      --index;
      entry.segment.index = index;
      return;
    }

    move_prev_leaf(entry, entry.leaf.pointer, entry.leaf.index);
  }

  static void move_prev_leaf(iterator_entry &entry, leaf_node *pointer,
                             size_type index) {
    if (index != 0) {
      --index;
      entry.leaf.index = index;
      entry.segment =
          find_last_segment(pointer->pointers[index], pointer->sizes[index]);
      return;
    }

    move_prev_branch(entry, pointer->parent_pointer, pointer->parent_index);
  }

  static void move_prev_branch(iterator_entry &entry, branch_node *pointer,
                               size_type index) {
    size_type height = 2;

    while (true) {
      if (index != 0) {
        entry = find_last_node(pointer->pointers[index - 1], height - 1);
        return;
      }

      index = pointer->parent_index;
      pointer = pointer->parent_pointer;
      ++height;
    }
  }

  // move_count
  static void move_iterator_count(iterator_data &it, difference_type diff) {
    it.pos += diff;
    if (diff > 0)
      move_next_segment_count(it.entry, it.entry.segment.index,
                              it.entry.segment.length, diff);
    else if (diff < 0)
      move_prev_segment_count(it.entry, it.entry.segment.index, -diff);
  }

  // move_next_count
  static void move_next_segment_count(iterator_entry &entry, size_type index,
                                      size_type length, size_type count) {
    index += count;
    if (index < length) {
      entry.segment.index = index;
      return;
    }

    // Special case for end iterator.
    if (entry.leaf.pointer == nullptr) return;

    move_next_leaf_count(entry, entry.leaf.pointer, entry.leaf.index,
                         index - length);
  }

  static void move_next_leaf_count(iterator_entry &entry, leaf_node *pointer,
                                   size_type index, size_type count) {
    while (true) {
      ++index;
      if (index == pointer->length) break;

      size_type size = pointer->sizes[index];
      if (size > count) {
        entry.leaf.index = index;
        entry.segment =
            find_index_segment(pointer->pointers[index], size, count);
        return;
      }
      count -= size;
    }

    // Special case for end iterator.
    if (pointer->parent_pointer == nullptr) {
      entry = find_end_leaf(pointer);
      return;
    }

    move_next_branch_count(entry, pointer->parent_pointer,
                           pointer->parent_index, count);
  }

  static void move_next_branch_count(iterator_entry &entry,
                                     branch_node *pointer, size_type index,
                                     size_type count) {
    size_type height = 2;

    while (true) {
      while (true) {
        ++index;
        if (index == pointer->length) break;

        size_type size = pointer->sizes[index];
        if (size > count) {
          entry = find_index_node(pointer->pointers[index], height - 1, count);
          return;
        }
        count -= size;
      }

      // Special case for end iterator.
      if (pointer->parent_pointer == nullptr) {
        entry = find_end_branch(pointer, height);
        return;
      }

      index = pointer->parent_index;
      pointer = pointer->parent_pointer;
      ++height;
    }
  }

  // move_prev_count
  static void move_prev_segment_count(iterator_entry &entry, size_type index,
                                      size_type count) {
    if (index >= count) {
      index -= count;
      entry.segment.index = index;
      return;
    }

    move_prev_leaf_count(entry, entry.leaf.pointer, entry.leaf.index,
                         count - index);
  }

  static void move_prev_leaf_count(iterator_entry &entry, leaf_node *pointer,
                                   size_type index, size_type count) {
    while (true) {
      if (index == 0) break;
      --index;

      size_type size = pointer->sizes[index];
      if (size >= count) {
        entry.leaf.index = index;
        entry.segment =
            find_index_segment(pointer->pointers[index], size, size - count);
        return;
      }
      count -= size;
    }

    move_prev_branch_count(entry, pointer->parent_pointer,
                           pointer->parent_index, count);
  }

  static void move_prev_branch_count(iterator_entry &entry,
                                     branch_node *pointer, size_type index,
                                     size_type count) {
    size_type height = 2;

    while (true) {
      while (true) {
        if (index == 0) break;
        --index;

        size_type size = pointer->sizes[index];
        if (size >= count) {
          entry = find_index_node(pointer->pointers[index], height - 1,
                                  size - count);
          return;
        }
        count -= size;
      }

      index = pointer->parent_index;
      pointer = pointer->parent_pointer;
      ++height;
    }
  }

  // move_segment
  static void move_after_segment(iterator_data it) {
    it.pos += it.segment.length - it.segment.index;
    move_next_leaf(it.entry);
  }

  static void move_after_segment_count(iterator_data it, size_type count) {
    it.pos += it.segment.length - it.segment.index + count;
    move_next_leaf_count(it.entry, count);
  }

  static void find_before_segment(iterator_data it) {
    it.pos -= it.segment.index + 1;
    move_prev_leaf(it.entry);
  }

  static void find_before_segment_count(iterator_data it, size_type count) {
    it.pos -= it.segment.index + 1 - count;
    move_prev_leaf_count(it.entry, count);
  }

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
    return element_traits::allocate(get_element_allocator(), segment_max);
  }

  node_pointer allocate_node() {
    return node_traits::allocate(get_node_allocator(), 1);
  }

  // deallocate
  void deallocate_segment(element_pointer pointer) {
    element_traits::deallocate(get_element_allocator(), pointer, segment_max);
  }

  void deallocate_node(node_pointer pointer) {
    node_traits::deallocate(get_node_allocator(), pointer, 1);
  }

  // delete
  void delete_segment(element_pointer pointer, size_type sz) {
    if (!is_trivial_)
      for (size_type i = 0, e = sz; i != e; ++i) destroy_element(pointer, i);
    deallocate_segment(pointer);
  }

  void delete_leaf(leaf_node *pointer) {
    for (size_type i = 0, e = pointer->length; i != e; ++i)
      delete_segment(pointer->pointers[i], pointer->sizes[i]);
  }

  void delete_node(node_pointer pointer, size_type height) {
    if (height == 1)
      delete_leaf(&pointer->leaf);
    else
      delete_branch(&pointer->branch, height);
    deallocate_node(pointer);
  }

  void delete_branch(branch_node *pointer, size_type height) {
    for (size_type i = 0, e = pointer->length; i != e; ++i)
      delete_node(pointer->pointers[i], height - 1);
  }

  void delete_root(void_pointer pointer, size_type sz, size_type height) {
    if (height == 0)
      delete_segment(cast_segment(pointer), sz);
    else
      delete_node(cast_node(pointer), height);
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

  size_type move_single_leaf(leaf_node *source, size_type source_index,
                             leaf_node *dest, size_type dest_index) {
    size_type sz = source->sizes[source_index];
    dest->sizes[dest_index] = sz;
    dest->pointers[dest_index] = source->pointers[source_index];
    return sz;
  }

  size_type move_single_branch(branch_node *source, size_type source_index,
                               branch_node *dest, size_type dest_index) {
    size_type sz = source->sizes[source_index];
    dest->sizes[dest_index] = sz;
    auto child = source->pointers[source_index];
    auto base = &child->base;
    base->parent_pointer = dest;
    base->parent_index = dest_index;
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
      size_type from = source_index;
      size_type last = source_index + count;
      size_type to = dest_index;

      while (from != last) {
        construct_element(dest, to, std::move(source[from]));
        destroy_element(source, from);
        ++from;
        ++to;
      }
    }
    return count;
  }

  size_type move_range_leaf(leaf_node *source, size_type source_index,
                            leaf_node *dest, size_type dest_index,
                            size_type count) {
    size_type copy_size = 0;
    size_type from = source_index;
    size_type last = source_index + count;
    size_type to = dest_index;

    while (from != last) {
      size_type sz = source->sizes[from];
      dest->sizes[to] = sz;
      dest->pointers[to] = source->pointers[from];
      copy_size += sz;
      ++from;
      ++to;
    }

    return copy_size;
  }

  size_type move_range_branch(branch_node *source, size_type source_index,
                              branch_node *dest, size_type dest_index,
                              size_type count) {
    size_type copy_size = 0;
    size_type from = source_index;
    size_type last = source_index + count;
    size_type to = dest_index;

    while (from != last) {
      size_type sz = source->sizes[from];
      dest->sizes[to] = sz;
      auto child = source->pointers[from];
      auto base = &child->base;
      base->parent_pointer = dest;
      base->parent_index = to;
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
      size_type first = index;
      size_type from = length;
      size_type to = length + distance;

      while (first != from) {
        --from;
        --to;
        construct_element(pointer, to, std::move(pointer[from]));
        destroy_element(pointer, from);
      }
    }
  }

  void move_forward_leaf(leaf_node *pointer, size_type length, size_type index,
                         size_type distance) {
    size_type first = index;
    size_type from = length;
    size_type to = length + distance;

    while (first != from) {
      --from;
      --to;
      pointer->sizes[to] = pointer->sizes[from];
      pointer->pointers[to] = pointer->pointers[from];
    }
  }

  void move_forward_branch(branch_node *pointer, size_type length,
                           size_type index, size_type distance) {
    size_type first = index;
    size_type from = length;
    size_type to = length + distance;

    while (first != from) {
      --from;
      --to;
      pointer->sizes[to] = pointer->sizes[from];
      auto child = pointer->pointers[from];
      auto base = &child->base;
      base->parent_index = to;
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
      size_type from = index + distance;
      size_type to = index;
      size_type last = length;

      while (to != last) {
        destroy_element(pointer, to);
        construct_element(pointer, to, std::move(pointer[from]));
        ++from;
        ++to;
      }
    }
  }

  void move_backward_leaf(leaf_node *pointer, size_type length, size_type index,
                          size_type distance) {
    size_type from = index + distance;
    size_type to = index;
    size_type last = length;

    while (to != last) {
      pointer->sizes[to] = pointer->sizes[from];
      pointer->pointers[to] = pointer->pointers[from];
      ++from;
      ++to;
    }
  }

  void move_backward_branch(branch_node *pointer, size_type length,
                            size_type index, size_type distance) {
    size_type from = index + distance;
    size_type to = index;
    size_type last = length;

    while (to != last) {
      pointer->sizes[to] = pointer->sizes[from];
      auto child = pointer->pointers[from];
      auto base = &child->base;
      base->parent_index = to;
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

  size_type copy_single_leaf(leaf_node *pointer, size_type index,
                             leaf_data child) {
    pointer->sizes[index] = child.size;
    pointer->pointers[index] = child.pointer;
    return child.size;
  }

  size_type copy_single_branch(branch_node *pointer, size_type index,
                               branch_data child) {
    pointer->sizes[index] = child.size;
    auto base = &child.pointer->base;
    base->parent_pointer = pointer;
    base->parent_index = index;
    pointer->pointers[index] = child.pointer;
    return child.size;
  }

  // delete_single

  // delete_range
  size_type delete_range_segment(element_pointer pointer, size_type index,
                                 size_type count) {
    if (!is_trivial_) {
      size_type from = index;
      size_type last = index + count;

      while (from != last) {
        destroy_element(pointer, from);
        ++from;
      }
    }
    return count;
  }

  size_type delete_range_leaf(leaf_node *pointer, size_type index,
                              size_type count) {
    size_type from = index;
    size_type last = index + count;
    size_type erase_size = 0;

    while (from != last) {
      auto sz = pointer->sizes[from];
      delete (pointer->pointers[from], sz);
      erase_size += sz;
      ++from;
    }

    return erase_size;
  }

  size_type delete_range_branch(branch_node *pointer, size_type index,
                                size_type count, size_type height) {
    size_type from = index;
    size_type last = index + count;
    size_type erase_size = 0;

    while (from != last) {
      auto sz = pointer->sizes[from];
      delete (pointer->pointers[from], sz, height - 1);
      erase_size += sz;
      ++from;
    }

    return erase_size;
  }

  // update_sizes
  void update_sizes_leaf(leaf_node *parent, size_type parent_index,
                         size_type update_size) {
    if (parent != nullptr) {
      parent->sizes[parent_index] += update_size;
      return update_sizes_branch(parent->parent_pointer, parent->parent_index,
                                 update_size);
    }

    get_size() += update_size;
  }

  void update_sizes_branch(branch_node *parent, size_type parent_index,
                           size_type update_size) {
    while (parent != nullptr) {
      parent->sizes[parent_index] += update_size;
      parent_index = parent->parent_index;
      parent = parent->parent_pointer;
    }

    get_size() += update_size;
  }

  // alloc_nodes_single
  node_pointer alloc_nodes_single(leaf_node *pointer,
                                  element_pointer segment_alloc) {
    node_pointer alloc = nullptr;
    try {
      // leaf
      if (pointer == nullptr) {
        alloc = allocate_node();
        return alloc;
      }

      if (pointer->length != base_max) return alloc;

      branch_node *branch_pointer = pointer->parent_pointer;
      alloc = allocate_node();
      alloc->branch.pointers[0] = nullptr;

      // branch
      while (true) {
        if (branch_pointer == nullptr) {
          auto temp = allocate_node();
          temp->branch.pointers[0] = alloc;
          alloc = temp;
          return alloc;
        }

        if (branch_pointer->length != base_max) return alloc;

        auto temp = allocate_node();
        temp->branch.pointers[0] = alloc;
        alloc = temp;

        branch_pointer = branch_pointer->parent_pointer;
      }
    } catch (...) {
      deallocate_segment(segment_alloc);

      while (alloc != nullptr) {
        auto temp = alloc->branch.pointers[0];
        deallocate_node(alloc);
        alloc = temp;
      }
      throw;
    }
  }

  // reserve_single
  iterator_data reserve_single_iterator(iterator_data it) {
    reserve_single_segment(it.entry, it.entry.segment.pointer,
                           it.entry.segment.index, it.entry.segment.length,
                           it.entry.leaf.pointer, it.entry.leaf.index);
    return it;
  }

  void reserve_single_segment(iterator_entry &entry, element_pointer pointer,
                              size_type index, size_type length,
                              leaf_node *parent_pointer,
                              size_type parent_index) {
    if (pointer == nullptr) {
      pointer = allocate_segment();
      get_root() = pointer;
      get_size() = 1;
      entry.segment.pointer = pointer;
      entry.segment.length = 1;
      return;
    }

    if (length != segment_max) {
      move_forward_segment(pointer, length, index, 1);
      ++entry.segment.length;
      update_sizes_leaf(parent_pointer, parent_index, 1);
      return;
    }

    element_pointer next_pointer = allocate_segment();
    auto alloc = alloc_nodes_single(parent_pointer, next_pointer);

    constexpr size_type sum = segment_max + 1;
    constexpr size_type pointer_length = sum / 2;
    constexpr size_type next_length = sum - pointer_length;

    if (index < pointer_length) {
      size_type left_index = pointer_length - 1;
      move_range_segment(pointer, left_index, next_pointer, 0, next_length);
      move_forward_segment(pointer, left_index, index, 1);
      entry.segment.length = pointer_length;
    } else {
      size_type new_index = index - pointer_length;
      size_type move_length = length - index;
      move_range_segment(pointer, pointer_length, next_pointer, 0, new_index);
      move_range_segment(pointer, index, next_pointer, new_index + 1,
                         move_length);
      entry.segment.length = next_length;
      entry.segment.pointer = next_pointer;
      entry.segment.index = new_index;
      ++entry.leaf.index;
    }

    reserve_single_leaf(entry, parent_pointer, parent_index + 1, alloc,
                        {next_pointer, next_length});
  }

  void reserve_single_leaf(iterator_entry &entry, leaf_node *pointer,
                           size_type index, node_pointer alloc,
                           leaf_data data) {
    if (pointer == nullptr) {
      pointer = &alloc->leaf;
      pointer->parent_pointer = nullptr;
      pointer->parent_index = 0;
      pointer->length = 2;
      copy_single_leaf(pointer, 0,
                       {cast_segment(get_root()), get_size() - data.size + 1});
      copy_single_leaf(pointer, 1, data);
      get_root() = alloc;
      ++get_height();
      ++get_size();
      entry.leaf.pointer = pointer;
      return;
    }

    pointer->sizes[index - 1] -= data.size - 1;

    if (pointer->length != base_max) {
      move_forward_leaf(pointer, pointer->length, index, 1);
      copy_single_leaf(pointer, index, data);
      ++pointer->length;
      update_sizes_branch(pointer->parent_pointer, pointer->parent_index, 1);
      return;
    }

    auto next_alloc = alloc->branch.pointers[0];
    auto next_pointer = &alloc->leaf;

    constexpr size_type sum = base_max + 1;
    constexpr size_type pointer_length = sum / 2;
    constexpr size_type alloc_length = sum - pointer_length;

    size_type next_size = 0;
    if (index < pointer_length) {
      size_type left_index = pointer_length - 1;
      next_size +=
          move_range_leaf(pointer, left_index, next_pointer, 0, alloc_length);
      move_forward_leaf(pointer, left_index, index, 1);
      copy_single_leaf(pointer, index, data);
    } else {
      size_type new_index = index - pointer_length;
      size_type move_length = pointer->length - index;
      next_size +=
          move_range_leaf(pointer, pointer_length, next_pointer, 0, new_index);
      next_size += move_range_leaf(pointer, index, next_pointer, new_index + 1,
                                   move_length);
      next_size += copy_single_leaf(next_pointer, new_index, data);
    }

    pointer->length = pointer_length;
    next_pointer->length = alloc_length;

    if (entry.leaf.index >= pointer_length) {
      entry.leaf.pointer = next_pointer;
      entry.leaf.index -= pointer_length;
    }

    reserve_single_branch(pointer->parent_pointer, pointer->parent_index + 1,
                          next_alloc, {alloc, next_size});
  }

  void reserve_single_branch(branch_node *pointer, size_type index,
                             node_pointer alloc, branch_data data) {
    while (true) {
      if (pointer == nullptr) {
        pointer = &alloc->branch;
        pointer->parent_pointer = nullptr;
        pointer->parent_index = 0;
        pointer->length = 2;
        copy_single_branch(pointer, 0,
                           {cast_node(get_root()), get_size() - data.size + 1});
        copy_single_branch(pointer, 1, data);
        get_root() = alloc;
        ++get_height();
        ++get_size();
        return;
      }

      pointer->sizes[index - 1] -= data.size - 1;

      if (pointer->length != base_max) {
        move_forward_branch(pointer, pointer->length, index, 1);
        copy_single_branch(pointer, index, data);
        ++pointer->length;
        update_sizes_branch(pointer->parent_pointer, pointer->parent_index, 1);
        return;
      }

      auto next_alloc = alloc->branch.pointers[0];
      auto next_pointer = &alloc->branch;

      constexpr size_type sum = base_max + 1;
      constexpr size_type pointer_length = sum / 2;
      constexpr size_type alloc_length = sum - pointer_length;

      size_type next_size = 0;
      if (index < pointer_length) {
        size_type left_index = pointer_length - 1;
        next_size += move_range_branch(pointer, left_index, next_pointer, 0,
                                       alloc_length);
        move_forward_branch(pointer, left_index, index, 1);
        copy_single_branch(pointer, index, data);
      } else {
        size_type new_index = index - pointer_length;
        size_type move_length = pointer->length - index;
        next_size += move_range_branch(pointer, pointer_length, next_pointer, 0,
                                       new_index);
        next_size += move_range_branch(pointer, index, next_pointer,
                                       new_index + 1, move_length);
        next_size += copy_single_branch(next_pointer, new_index, data);
      }

      pointer->length = pointer_length;
      next_pointer->length = alloc_length;

      data = {alloc, next_size};
      index = pointer->parent_index + 1;
      pointer = pointer->parent_pointer;
      alloc = next_alloc;
    }
  }

  // erase_single
  iterator_data erase_single_iterator(iterator_data it) {
    destroy_element(it.entry.segment.pointer, it.entry.segment.index);
    erase_single_segment(it.entry, it.entry.segment.pointer,
                         it.entry.segment.index, it.entry.segment.length,
                         it.entry.leaf.pointer, it.entry.leaf.index);
    if (it.entry.segment.index == it.entry.segment.length)
      move_next_leaf(it.entry, it.entry.leaf.pointer, it.entry.leaf.index);
    return it;
  }

  void erase_single_segment(iterator_entry &entry, element_pointer pointer,
                            size_type index, size_type length,
                            leaf_node *parent_pointer, size_type parent_index) {
    if (length == 1) {
      get_root() = nullptr;
      deallocate_segment(cast_segment(get_root()));
      --get_size();
      entry.segment.pointer = nullptr;
      entry.segment.index = 0;
      entry.segment.length = 0;
      return;
    }

    if (length != segment_min || parent_pointer == nullptr) {
      --length;
      move_backward_segment(pointer, length, index, 1);
      entry.segment.length = length;
      update_sizes_leaf(parent_pointer, parent_index, -1);
      return;
    }

    constexpr size_type merge_size = segment_max - 1;
    auto pointers = &parent_pointer->pointers[0];
    auto sizes = &parent_pointer->sizes[0];
    --length;

    size_type erase_index;
    if (parent_index != 0) {
      auto prev_index = parent_index - 1;
      auto prev_pointer = pointers[prev_index];
      auto prev_length = sizes[prev_index];

      if (prev_length != segment_min) {
        --prev_length;
        move_forward_segment(pointer, index, 0, 1);
        move_single_segment(prev_pointer, prev_length, pointer, 0);
        sizes[prev_index] = prev_length;
        ++entry.segment.index;
        update_sizes_branch(parent_pointer->parent_pointer,
                            parent_pointer->parent_index, -1);
        return;
      }

      move_range_segment(pointer, 0, prev_pointer, prev_length, index);
      move_range_segment(pointer, index + 1, prev_pointer, prev_length + index,
                         length - index);
      sizes[prev_index] = merge_size;
      erase_index = parent_index;
      entry.segment.pointer = prev_pointer;
      entry.segment.length = merge_size;
      entry.segment.index += segment_min;
      --entry.leaf.index;
    }

    else {
      auto next_index = parent_index + 1;
      auto next_pointer = pointers[next_index];
      auto next_length = sizes[next_index];

      if (next_length != segment_min) {
        --next_length;
        move_backward_segment(pointer, length, index, 1);
        move_single_segment(next_pointer, 0, pointer, length);
        move_backward_segment(next_pointer, next_length, 0, 1);
        sizes[next_index] = next_length;
        update_sizes_branch(parent_pointer->parent_pointer,
                            parent_pointer->parent_index, -1);
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

  void erase_single_leaf(leaf_entry &entry, leaf_node *pointer,
                         size_type index) {
    deallocate_segment(pointer->pointers[index]);

    auto parent_pointer = pointer->parent_pointer;
    auto parent_index = pointer->parent_index;
    auto length = pointer->length;

    if (pointer->length == 2) {
      auto other = pointer->pointers[index ^ 1];
      deallocate_node(cast_node(get_root()));
      get_root() = other;
      --get_size();
      --get_height();
      entry.pointer = nullptr;
      entry.index = 0;
      return;
    }

    if (length != base_min || parent_pointer == nullptr) {
      --length;
      move_backward_leaf(pointer, length, index, 1);
      pointer->length = length;
      update_sizes_branch(parent_pointer, parent_index, -1);
      return;
    }

    auto pointers = &parent_pointer->pointers[0];
    auto sizes = &parent_pointer->sizes[0];
    --length;

    size_type erase_index;
    if (parent_index != 0) {
      auto prev_index = parent_index - 1;
      auto prev_pointer = &pointers[prev_index]->leaf;
      auto prev_length = prev_pointer->length;

      if (prev_length != base_min) {
        --prev_length;
        move_forward_leaf(pointer, index, 0, 1);
        auto sz = move_single_leaf(prev_pointer, prev_length, pointer, 0);
        sizes[prev_index] -= sz;
        sizes[parent_index] += sz - 1;
        prev_pointer->length = prev_length;
        ++entry.index;
        update_sizes_branch(parent_pointer->parent_pointer,
                            parent_pointer->parent_index, -1);

        return;
      }

      auto sz = move_range_leaf(pointer, 0, prev_pointer, prev_length, index);
      sz += move_range_leaf(pointer, index + 1, prev_pointer,
                            prev_length + index, length - index);
      prev_pointer->length += length;
      sizes[prev_index] += sz;
      erase_index = parent_index;
      entry.pointer = prev_pointer;
      entry.index += prev_length;
    }

    else {
      auto next_index = parent_index + 1;
      auto next_pointer = &pointers[next_index]->leaf;
      auto next_length = next_pointer->length;

      if (next_length != base_min) {
        --next_length;
        move_backward_leaf(pointer, length, index, 1);
        auto sz = move_single_leaf(next_pointer, 0, pointer, length);
        move_backward_leaf(next_pointer, next_length, 0, 1);
        sizes[next_index] -= sz;
        sizes[parent_index] += sz - 1;
        next_pointer->length = next_length;
        update_sizes_branch(parent_pointer->parent_pointer,
                            parent_pointer->parent_index, -1);

        return;
      }

      move_backward_leaf(pointer, length, index, 1);
      auto sz = move_range_leaf(next_pointer, 0, pointer, length, next_length);
      pointer->length += next_length - 1;
      sizes[parent_index] += sz - 1;
      erase_index = next_index;
    }

    erase_single_branch(parent_pointer, erase_index);
  }

  void erase_single_branch(branch_node *pointer, size_type index) {
    while (true) {
      deallocate_node(pointer->pointers[index]);

      auto parent_pointer = pointer->parent_pointer;
      auto parent_index = pointer->parent_index;
      auto length = pointer->length;

      if (length == 2) {
        auto other = pointer->pointers[index ^ 1];
        auto base = &other->base;
        deallocate_node(cast_node(get_root()));
        get_root() = other;
        base->parent_pointer = nullptr;
        base->parent_index = 0;
        --get_size();
        --get_height();

        return;
      }

      if (length != base_min || parent_pointer == nullptr) {
        --length;
        move_backward_branch(pointer, length, index, 1);
        pointer->length = length;
        update_sizes_branch(parent_pointer, parent_index, -1);
        return;
      }

      --length;
      auto pointers = &parent_pointer->pointers[0];
      auto sizes = &parent_pointer->sizes[0];

      size_type erase_index;
      if (parent_index != 0) {
        auto prev_index = parent_index - 1;
        auto prev_pointer = &pointers[prev_index]->branch;
        auto prev_length = prev_pointer->length;

        if (prev_length != base_min) {
          --prev_length;
          move_forward_branch(pointer, index, 0, 1);
          auto sz = move_single_branch(prev_pointer, prev_length, pointer, 0);
          sizes[prev_index] -= sz;
          sizes[parent_index] += sz - 1;
          prev_pointer->length = prev_length;
          update_sizes_branch(parent_pointer->parent_pointer,
                              parent_pointer->parent_index, -1);

          return;
        }

        auto sz =
            move_range_branch(pointer, 0, prev_pointer, prev_length, index);
        sz += move_range_branch(pointer, index + 1, prev_pointer,
                                prev_length + index, length - index);
        prev_pointer->length += length;
        sizes[prev_index] += sz;
        erase_index = parent_index;
      }

      else {
        auto next_index = parent_index + 1;
        auto next_pointer = &pointers[next_index]->branch;
        auto next_length = next_pointer->length;

        if (next_length != base_min) {
          --next_length;
          move_backward_branch(pointer, length, index, 1);
          auto sz = move_single_branch(next_pointer, 0, pointer, length);
          move_backward_branch(next_pointer, next_length, 0, 1);
          sizes[next_index] -= sz;
          sizes[parent_index] += sz - 1;
          next_pointer->length = next_length;
          update_sizes_branch(parent_pointer->parent_pointer,
                              parent_pointer->parent_index, -1);

          return;
        }

        move_backward_branch(pointer, length, index, 1);
        auto sz =
            move_range_branch(next_pointer, 0, pointer, length, next_length);
        pointer->length += next_length - 1;
        sizes[parent_index] += sz - 1;
        erase_index = next_index;
      }

      pointer = parent_pointer;
      index = erase_index;
    }
  }

  // helpers
  void deallocate() { delete_root(get_root(), get_size(), get_height()); }

  void steal(segmented_tree_seq &other) {
    get_root() = other.get_root();
    get_height() = other.get_height();
    get_size() = other.get_size();
    other.get_root() = nullptr;
    other.get_height() = 0;
    other.get_size() = 0;
  }

  // debug functions
  void verify_iterator(iterator_data a, size_type pos) {
#ifdef SEGMENTED_TREE_SEQ_DEBUG
    auto b = position(pos).it_;
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
#endif
  }

 public:
  // public interface
  explicit segmented_tree_seq(Allocator const &alloc = Allocator())
      : data_{nullptr, 0, 0, alloc, alloc} {}

  segmented_tree_seq(size_type count, T const &value,
                     Allocator const &alloc = Allocator())
      : segmented_tree_seq{alloc} {
    insert(end(), count, value);
  }

  explicit segmented_tree_seq(size_type count) : segmented_tree_seq{} {}

  template <class InputIt>
  segmented_tree_seq(InputIt first, InputIt last,
                     Allocator const &alloc = Allocator())
      : segmented_tree_seq{alloc} {
    insert(end(), first, last);
  }

  segmented_tree_seq(segmented_tree_seq const &other)
      : segmented_tree_seq{
            element_traits::select_on_container_copy_construction(
                other.get_element_allocator())} {
    insert(end(), other.begin(), other.end());
  }

  segmented_tree_seq(segmented_tree_seq const &other, Allocator const &alloc)
      : segmented_tree_seq{alloc} {
    insert(end(), other.begin(), other.end());
  }

  segmented_tree_seq(segmented_tree_seq &&other)
      : segmented_tree_seq{std::move(other.get_element_allocator())} {
    steal(other);
  }

  segmented_tree_seq(segmented_tree_seq &&other, Allocator const &alloc)
      : segmented_tree_seq{alloc} {
    if (get_element_allocator() == other.get_element_allocator())
      steal(other);
    else
      insert(end(), std::make_move_iterator(other.begin()),
             std::make_move_iterator(other.end()));
  }

  segmented_tree_seq(std::initializer_list<T> init,
                     Allocator const &alloc = Allocator())
      : segmented_tree_seq{alloc} {
    insert(end(), init.begin(), init.end());
  }

  ~segmented_tree_seq() { deallocate(); }

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

  segmented_tree_seq &operator=(segmented_tree_seq &&other) {
    if (element_traits::propagate_on_container_move_assignment::value ||
        get_element_allocator() == other.get_element_allocator()) {
      deallocate();
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

  template <class InputIt>
  void assign(InputIt first, InputIt last) {
    auto first2 = begin();
    auto last2 = end();
    while (true) {
      if (first == last) {
        erase(first2, last2);
        return;
      }
      if (first2 == last2) {
        insert(last2, first, last);
        return;
      }

      *first2 = *first;
      ++first2;
      ++first;
    }
  }

  void assign(std::initializer_list<T> ilist) {
    assign(ilist.begin(), ilist.end());
  }

  allocator_type get_allocator() const { return get_element_allocator(); }

  reference at(size_type pos) {
    if (pos > size())
      throw std::out_of_range{"segmented_tree_seq at() out of bounds"};
    return (*this)[pos];
  }

  const_reference at(size_type pos) const {
    if (pos > size())
      throw std::out_of_range{"segmented_tree_seq at() out of bounds"};
    return (*this)[pos];
  }

  reference operator[](size_type pos) {
    iterator it = find_index_root(get_root(), get_size(), get_height(), pos);
    return *it;
  }

  const_reference operator[](size_type pos) const {
    iterator it = find_index_root(get_root(), get_size(), get_height(), pos);
    return *it;
  }

  reference front() { return *begin(); }

  const_reference front() const { return *begin(); }

  reference back() { return *penultimate(); }

  const_reference back() const { return *penultimate(); }

  iterator begin() {
    return find_first_root(get_root(), get_size(), get_height());
  }

  const_iterator begin() const {
    return find_first_root(get_root(), get_size(), get_height());
  }

  const_iterator cbegin() const { return begin(); }

  iterator penultimate() {
    return find_last_root(get_root(), get_size(), get_height());
  }

  const_iterator penultimate() const {
    return find_last_root(get_root(), get_size(), get_height());
  }

  const_iterator cpenultimate() const { return penultimate(); }

  iterator end() { return find_end_root(get_root(), get_size(), get_height()); }

  const_iterator end() const {
    return find_end_root(get_root(), get_size(), get_height());
  }

  const_iterator cend() const { return end(); }

  reverse_iterator rbegin() { return std::reverse_iterator<iterator>{end()}; }

  const_reverse_iterator rbegin() const {
    return std::reverse_iterator<iterator>{end()};
  }

  const_reverse_iterator crbegin() const { return rbegin(); }

  reverse_iterator rend() { return std::reverse_iterator<iterator>{begin()}; }

  const_reverse_iterator rend() const {
    return std::reverse_iterator<iterator>{begin()};
  }

  const_reverse_iterator crend() const { return rend(); }

  iterator position(size_type pos) {
    assert(pos <= size());
    if (pos >= size()) return end();
    return find_index_root(get_root(), get_size(), get_height(), pos);
  }

  const_iterator cposition(size_type pos) {
    assert(pos <= size());
    if (pos >= size()) return end();
    return find_index_root(get_root(), get_size(), get_height(), pos);
  }

  reverse_iterator rposition(size_type pos) {
    return reverse_iterator{position(size() - pos)};
  }

  const_reverse_iterator crposition(size_type pos) {
    return const_reverse_iterator{position(size() - pos)};
  }

  bool empty() const { return get_size() == 0; }

  size_type size() const { return get_size(); }

  size_type max_size() const { return std::numeric_limits<size_type>::max(); }

  void clear() {
    deallocate();
    get_root() = nullptr;
    get_height() = 0;
    get_size() = 0;
  }

  iterator insert(const_iterator pos, T const &value) {
    return emplace(pos, value);
  }

  iterator insert(const_iterator pos, T &&value) {
    return emplace(pos, std::forward<T>(value));
  }

  iterator insert(const_iterator pos, size_type count, T const &value) {
    while (count != 0) {
      pos = insert(pos, value);
      ++pos;
    }
    pos -= count;
    return pos;
  }

  template <class InputIt>
  iterator insert(const_iterator pos, InputIt first, InputIt last) {
    size_type count = 0;
    while (first != last) {
      pos = insert(pos, *first);
      ++first;
      ++pos;
      ++count;
    }
    pos -= count;
    return pos.it_;
  }

  iterator insert(const_iterator pos, std::initializer_list<T> ilist) {
    return insert(pos, ilist.begin(), ilist.end());
  }

  template <class... Args>
  iterator emplace(const_iterator pos, Args &&... args) {
    auto it = reserve_single_iterator(pos.it_);
    try {
      copy_single_segment(it.entry.segment.pointer, it.entry.segment.index,
                          std::forward<Args>(args)...);
    } catch (...) {
      erase_single_segment(it.entry, it.entry.segment.pointer,
                           it.entry.segment.index, it.entry.segment.length,
                           it.entry.leaf.pointer, it.entry.leaf.index);
      throw;
    }

    verify_iterator(it, pos.it_.pos);
    return it;
  }

  iterator erase(const_iterator pos) {
    auto it = erase_single_iterator(pos.it_);
    verify_iterator(it, pos.it_.pos);
    return it;
  }

  iterator erase(const_iterator first, const_iterator last) {
    while (first != last) {
      --last;
      last = erase(last);
    }
    return last.it_;
  }

  void push_back(T const &value) { emplace_back(value); }

  void push_back(T &&value) { emplace_back(std::move<T>(value)); }

  template <class... Args>
  void emplace_back(Args &&... args) {
    emplace(end(), std::forward<Args>(args)...);
  }

  void pop_back() { erase(penultimate()); }

  void push_front(const T &value) { emplace_front(value); }

  void push_front(T &&value) { emplace_front(std::move<T>(value)); }

  template <class... Args>
  void emplace_front(Args &&... args) {
    emplace(begin(), std::forward<Args>(args)...);
  }

  void pop_front() { erase(begin()); }

  void resize(size_type count) { resize(count, {}); }

  void resize(size_type count, value_type const &value) {
    auto sz = size();
    if (sz == count) return;

    auto last = end();
    if (count < sz)
      erase(position(count), last);
    else
      insert(last, count, value);
  }

  void swap(segmented_tree_seq &other) {
    using std::swap;
    swap(get_root(), other.get_root());
    swap(get_height(), other.get_height());
    swap(get_size(), other.get_size());
    if (element_traits::propagate_on_container_swap::value) {
      swap(get_element_allocator(), other.get_element_allocator());
      swap(get_node_allocator(), other.get_node_allocator());
    }
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

  void remove(const T &value) {
    erase(std::remove(begin(), end(), value), end());
  }

  template <class UnaryPredicate>
  void remove_if(UnaryPredicate p) {
    erase(std::remove_if(begin(), end(), p), end());
  }

  void reverse() { std::reverse(begin(), end()); }

  void unique() { unique(std::less<value_type>{}); }

  template <class BinaryPredicate>
  void unique(BinaryPredicate p) {
    erase(std::unique(begin(), end(), p), end());
  }

  void sort() { sort(std::less<value_type>{}); }

  template <class Compare>
  void sort(Compare comp) {
    std::stable_sort(begin(), end());
  }
};

// free functions
template <typename T, typename Alloc, size_t... Args>
bool operator==(segmented_tree_seq<T, Alloc, Args...> &lhs,
                segmented_tree_seq<T, Alloc, Args...> &rhs) {
  return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename T, typename Alloc, size_t... Args>
bool operator!=(segmented_tree_seq<T, Alloc, Args...> &lhs,
                segmented_tree_seq<T, Alloc, Args...> &rhs) {
  return !(lhs == rhs);
}

template <typename T, typename Alloc, size_t... Args>
bool operator<(segmented_tree_seq<T, Alloc, Args...> &lhs,
               segmented_tree_seq<T, Alloc, Args...> &rhs) {
  return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(),
                                      rhs.end());
}

template <typename T, typename Alloc, size_t... Args>
bool operator<=(segmented_tree_seq<T, Alloc, Args...> &lhs,
                segmented_tree_seq<T, Alloc, Args...> &rhs) {
  return !(rhs < lhs);
}

template <typename T, typename Alloc, size_t... Args>
bool operator>(segmented_tree_seq<T, Alloc, Args...> &lhs,
               segmented_tree_seq<T, Alloc, Args...> &rhs) {
  return rhs < lhs;
}

template <typename T, typename Alloc, size_t... Args>
bool operator>=(segmented_tree_seq<T, Alloc> &lhs,
                segmented_tree_seq<T, Alloc> &rhs) {
  return !(lhs < rhs);
}

template <typename T, typename Traits, typename Alloc, size_t... Args>
void swap(segmented_tree_seq<T, Alloc, Args...> &a,
          segmented_tree_seq<T, Alloc, Args...> &b) {
  a.swap(b);
}

template <typename T, typename Traits, typename Alloc, size_t... Args>
std::ostream &operator<<(std::basic_ostream<T, Traits> &out,
                         segmented_tree_seq<T, Alloc, Args...> &tree) {
  auto first = tree.begin();
  auto last = tree.end();

  while (first.begin() != last.begin()) {
    out.write(first.index(), first.end() - first.index());
    first.move_after_segment();
  }
  if (first.index() != last.index()) {
    out.write(first.index(), last.index() - first.index());
  }

  return out;
}
}
}
