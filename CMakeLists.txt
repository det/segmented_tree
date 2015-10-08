cmake_minimum_required (VERSION 2.8)
project (segmented_tree_seq)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set (CMAKE_CXX_FLAGS "-std=c++11 -Weverything -Wno-c++98-compat \
                        -Wno-c++98-compat-pedantic -Wno-padded")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set (CMAKE_CXX_FLAGS "-std=c++11 -Wpedantic -Wall -Wextra \
                        -Wno-missing-field-initializers")
endif ()

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  add_definitions(-DBOOST_SEGMENTED_TREE_SEQ_DEBUG)
endif ()


find_package(Boost)

include_directories (include)
include_directories (SYSTEM submodule)
include_directories (SYSTEM ${Boost_INCLUDE_DIRS})
add_subdirectory (include/boost)
add_subdirectory (example)
