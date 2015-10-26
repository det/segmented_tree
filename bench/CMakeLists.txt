add_custom_target(headers SOURCES common.hpp iterator.hpp range.hpp single.hpp)

add_executable (single_segmented_tree_seq_8
                single_segmented_tree_seq_8.cpp)
add_executable (single_btree_seq_8
                single_btree_seq_8.cpp)
add_executable (single_avl_array_8
                single_avl_array_8.cpp)
add_executable (single_bpt_sequence_8
                single_bpt_sequence_8.cpp)
add_executable (single_vector_8
                single_vector_8.cpp)
add_executable (single_deque_8
                single_deque_8.cpp)

add_executable (single_segmented_tree_seq_64
                single_segmented_tree_seq_64.cpp)
add_executable (single_btree_seq_64
                single_btree_seq_64.cpp)
add_executable (single_avl_array_64
                single_avl_array_64.cpp)
add_executable (single_bpt_sequence_64
                single_bpt_sequence_64.cpp)
add_executable (single_vector_64
                single_vector_64.cpp)
add_executable (single_deque_64
                single_deque_64.cpp)

add_executable (range_segmented_tree_seq_8
                range_segmented_tree_seq_8.cpp)
add_executable (range_btree_seq_8
                range_btree_seq_8.cpp)
add_executable (range_avl_array_8
                range_avl_array_8.cpp)
add_executable (range_bpt_sequence_8
                range_bpt_sequence_8.cpp)
add_executable (range_vector_8
                range_vector_8.cpp)
add_executable (range_deque_8
                range_deque_8.cpp)

add_executable (range_segmented_tree_seq_64
                range_segmented_tree_seq_64.cpp)
add_executable (range_btree_seq_64
                range_btree_seq_64.cpp)
add_executable (range_avl_array_64
                range_avl_array_64.cpp)
add_executable (range_bpt_sequence_64
                range_bpt_sequence_64.cpp)
add_executable (range_vector_64
                range_vector_64.cpp)
add_executable (range_deque_64
                range_deque_64.cpp)
