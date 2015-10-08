// (C) Copyright Chris Clearwater 2014-2015. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy
// at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_SEGMENTED_TREE_SEQ_FWD
#define BOOST_SEGMENTED_TREE_SEQ_FWD

#include <memory>

namespace boost {

template <typename T, typename Allocator = std::allocator<T>,
          std::size_t segment_target = 512, std::size_t base_target = 512>
class segmented_tree_seq;
}

#endif  // #ifndef BOOST_SEGMENTED_TREE_SEQ_FWD
