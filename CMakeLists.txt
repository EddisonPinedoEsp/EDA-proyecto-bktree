cmake_minimum_required(VERSION 3.14)
project(bktree CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
add_compile_options(-Wall -Wextra)

add_library(bktree_lib
  src/core/text_utils.cpp
  src/core/levenshtein.cpp
  src/baseline/exhaustive.cpp
  src/bktree/bktree.cpp)
target_include_directories(bktree_lib PUBLIC src)

enable_testing()
foreach(t test_levenshtein test_bktree)
  add_executable(${t} tests/${t}.cpp)
  target_link_libraries(${t} bktree_lib)
  add_test(NAME ${t} COMMAND ${t})
endforeach()
