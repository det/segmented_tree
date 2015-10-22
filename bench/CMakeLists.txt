add_custom_target(headers SOURCES bench_common.hpp bench_range.hpp
                  bench_single.hpp compile.hpp random_data_common.hpp
                  random_data_range.hpp random_data_single.hpp)

add_executable (test_compile_segmented_tree_seq
                test_compile_segmented_tree_seq.cpp)

add_executable (gen_random_data_single_8 gen_random_data_single_8.cpp)
add_executable (gen_random_data_single_64 gen_random_data_single_64.cpp)

add_executable (bench_single_segmented_tree_seq_8
                bench_single_segmented_tree_seq_8.cpp)
add_executable (bench_single_btree_seq_8
                bench_single_btree_seq_8.cpp)
add_executable (bench_single_avl_array_8
                bench_single_avl_array_8.cpp)
add_executable (bench_single_bpt_sequence_8
                bench_single_bpt_sequence_8.cpp)
add_executable (bench_single_vector_8
                bench_single_vector_8.cpp)
add_executable (bench_single_deque_8
                bench_single_deque_8.cpp)

add_executable (bench_single_segmented_tree_seq_64
                bench_single_segmented_tree_seq_64.cpp)
add_executable (bench_single_btree_seq_64
                bench_single_btree_seq_64.cpp)
add_executable (bench_single_avl_array_64
                bench_single_avl_array_64.cpp)
add_executable (bench_single_bpt_sequence_64
                bench_single_bpt_sequence_64.cpp)
add_executable (bench_single_vector_64
                bench_single_vector_64.cpp)
add_executable (bench_single_deque_64
                bench_single_deque_64.cpp)

add_executable (gen_random_data_range_8 gen_random_data_range_8.cpp)
add_executable (gen_random_data_range_64 gen_random_data_range_64.cpp)

add_executable (bench_range_segmented_tree_seq_8
                bench_range_segmented_tree_seq_8.cpp)
add_executable (bench_range_btree_seq_8
                bench_range_btree_seq_8.cpp)
add_executable (bench_range_avl_array_8
                bench_range_avl_array_8.cpp)
add_executable (bench_range_bpt_sequence_8
                bench_range_bpt_sequence_8.cpp)
add_executable (bench_range_vector_8
                bench_range_vector_8.cpp)
add_executable (bench_range_deque_8
                bench_range_deque_8.cpp)

add_executable (bench_range_segmented_tree_seq_64
                bench_range_segmented_tree_seq_64.cpp)
add_executable (bench_range_btree_seq_64
                bench_range_btree_seq_64.cpp)
add_executable (bench_range_avl_array_64
                bench_range_avl_array_64.cpp)
add_executable (bench_range_bpt_sequence_64
                bench_range_bpt_sequence_64.cpp)
add_executable (bench_range_vector_64
                bench_range_vector_64.cpp)
add_executable (bench_range_deque_64
                bench_range_deque_64.cpp)
