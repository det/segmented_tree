set (random_data_single_headers
  random_data_single.hpp)

set (bench_single_headers
  ${random_data_single_headers}
  adler64.hpp
  bench_single.hpp)

add_executable (test_compile_segmented_tree_seq ${compile_headers} test_compile_segmented_tree_seq.cpp)

add_executable (gen_random_data_single_8 ${random_data_single_headers} gen_random_data_single_8.cpp)
add_executable (gen_random_data_single_64 ${random_data_single_headers} gen_random_data_single_64.cpp)

add_executable (bench_single_segmented_tree_seq_64 ${bench_single_headers} bench_single_segmented_tree_seq_64.cpp)
add_executable (bench_single_btree_seq_64 ${bench_single_headers} bench_single_btree_seq_64.cpp)
add_executable (bench_single_avl_array_64 ${bench_single_headers} bench_single_avl_array_64.cpp)
add_executable (bench_single_bpt_sequence_64 ${bench_single_headers} bench_single_bpt_sequence_64.cpp)
add_executable (bench_single_vector_64 ${bench_single_headers} bench_single_vector_64.cpp)
add_executable (bench_single_deque_64 ${bench_single_headers} bench_single_deque_64.cpp)


add_executable (bench_single_segmented_tree_seq_8 ${bench_single_headers}  bench_single_segmented_tree_seq_8.cpp)
add_executable (bench_single_btree_seq_8 ${bench_single_headers} bench_single_btree_seq_8.cpp)
add_executable (bench_single_avl_array_8 ${bench_single_headers} bench_single_avl_array_8.cpp)
add_executable (bench_single_bpt_sequence_8 ${bench_single_headers} bench_single_bpt_sequence_8.cpp)
add_executable (bench_single_vector_8 ${bench_single_headers} bench_single_vector_8.cpp)
add_executable (bench_single_deque_8 ${bench_single_headers} bench_single_deque_8.cpp)


