add_executable (test_sequence_0 test_sequence.cpp)
target_link_libraries(test_sequence_0 boost_unit_test_framework)
set_property(TARGET test_sequence_0 PROPERTY COMPILE_DEFINITIONS
             BOOST_TEST_DYN_LINK TARGET_SIZE=0)

add_executable (test_sequence_512 test_sequence.cpp)
target_link_libraries(test_sequence_512 boost_unit_test_framework)
set_property(TARGET test_sequence_512 PROPERTY COMPILE_DEFINITIONS
             BOOST_TEST_DYN_LINK TARGET_SIZE=512)
