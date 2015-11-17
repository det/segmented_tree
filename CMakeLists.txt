cmake_minimum_required (VERSION 2.8)
project (segmented_tree)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set (CMAKE_CXX_FLAGS "-std=c++11 -Weverything -Wno-c++98-compat \
                        -Wno-c++98-compat-pedantic -Wno-padded \
                        -Wno-disabled-macro-expansion -Wno-global-constructors")
  set (CMAKE_CXX_FLAGS_DEBUG "-O1 -g -fsanitize=address -fsanitize=undefined")
  set (CMAKE_CXX_FLAGS_RELEASE "-O3 -flto")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set (CMAKE_CXX_FLAGS "-std=c++11 -Wpedantic -Wall -Wextra \
                        -Wno-missing-field-initializers")
  set (CMAKE_CXX_FLAGS_DEBUG "-Og -g")
  set (CMAKE_CXX_FLAGS_RELEASE "-O2 -flto")
endif ()

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  add_definitions(-DBOOST_SEGMENTED_TREE_DEBUG)
endif ()


find_package(Boost)
include_directories (include)
include_directories (SYSTEM submodule)
include_directories (SYSTEM ${Boost_INCLUDE_DIRS})
add_subdirectory (include/boost/segmented_tree)
add_subdirectory (bench)
add_subdirectory (common)
add_subdirectory (test)
