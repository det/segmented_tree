add_custom_target(common_headers SOURCES common.hpp generate_range.hpp
                  generate_single.hpp iterator.hpp range.hpp single.hpp)

add_executable (generate_single_8 generate_single_8.cpp)
add_executable (generate_single_64 generate_single_64.cpp)
add_executable (generate_range_8 generate_range_8.cpp)
add_executable (generate_range_64 generate_range_64.cpp)
