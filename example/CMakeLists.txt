set (bench_headers
	random_data.hpp
	adler64.hpp
	bench.hpp)

set (compile_headers
        compile.hpp)

add_executable (gen_random_data_64 random_data.hpp gen_random_data_64.cpp)
add_executable (bench_segmented_tree_seq_64 ${bench_headers} bench_segmented_tree_seq_64.cpp)
add_executable (bench_btree_seq_64 ${bench_headers} bench_btree_seq_64.cpp)
add_executable (bench_avl_array_64 ${bench_headers} bench_avl_array_64.cpp)
add_executable (bench_bpt_sequence_64 ${bench_headers} bench_bpt_sequence_64.cpp)
add_executable (bench_vector_64 ${bench_headers} bench_vector_64.cpp)
add_executable (bench_deque_64 ${bench_headers} bench_deque_64.cpp)
add_executable (gen_random_data_8 random_data.hpp gen_random_data_8.cpp)
add_executable (bench_segmented_tree_seq_8 ${bench_headers}  bench_segmented_tree_seq_8.cpp)
add_executable (bench_btree_seq_8 ${bench_headers} bench_btree_seq_8.cpp)
add_executable (bench_avl_array_8 ${bench_headers} bench_avl_array_8.cpp)
add_executable (bench_bpt_sequence_8 ${bench_headers} bench_bpt_sequence_8.cpp)
add_executable (bench_vector_8 ${bench_headers} bench_vector_8.cpp)
add_executable (bench_deque_8 ${bench_headers} bench_deque_8.cpp)

add_executable (test_compile_segmented_tree_seq ${compile_headers} test_compile_segmented_tree_seq.cpp)
