#include "compile.hpp"
#include "boost/segmented_tree_seq.hpp"

int main() {
  test_compile<boost::segmented_tree_seq<std::uint64_t>,
               std::uint64_t>();
}
