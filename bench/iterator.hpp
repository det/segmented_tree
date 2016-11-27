#ifndef BENCH_ITERATOR
#define BENCH_ITERATOR

#include <stdexcept>
#include <type_traits>
#include <vector>
#include "../common/iterator.hpp"
#include "common.hpp"

template <typename Container, typename T>
void bench_iterator(Container const& container, std::vector<T> const& data) {
  verify(accumulate_forward(data), bench("Accumulate forward", [&] {
           return accumulate_forward(container);
         }));

  verify(accumulate_forward_by(data, 1), bench("Accumulate forward by 1", [&] {
           return accumulate_forward_by(container, 1);
         }));

  verify(accumulate_forward_by(data, 10),
         bench("Accumulate forward by 10",
               [&] { return accumulate_forward_by(container, 10); }));

  verify(accumulate_forward_by(data, 100),
         bench("Accumulate forward by 100",
               [&] { return accumulate_forward_by(container, 100); }));

  verify(accumulate_forward_by(data, 1000),
         bench("Accumulate forward by 1000",
               [&] { return accumulate_forward_by(container, 1000); }));

  verify(accumulate_forward_by(data, 10000),
         bench("Accumulate forward by 10000",
               [&] { return accumulate_forward_by(container, 10000); }));

  verify(accumulate_backward(data), bench("Accumulate backward", [&] {
           return accumulate_backward(container);
         }));

  verify(accumulate_backward_by(data, 1),
         bench("Accumulate backward by 1",
               [&] { return accumulate_backward_by(container, 1); }));

  verify(accumulate_backward_by(data, 10),
         bench("Accumulate backward by 10",
               [&] { return accumulate_backward_by(container, 10); }));

  verify(accumulate_backward_by(data, 100),
         bench("Accumulate backward by 100",
               [&] { return accumulate_backward_by(container, 100); }));

  verify(accumulate_backward_by(data, 1000),
         bench("Accumulate backward by 1000",
               [&] { return accumulate_backward_by(container, 1000); }));

  verify(accumulate_backward_by(data, 10000),
         bench("Accumulate backward by 10000",
               [&] { return accumulate_backward_by(container, 10000); }));
}

#endif  // #ifndef BENCH_ITERATOR
