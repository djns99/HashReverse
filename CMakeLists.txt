cmake_minimum_required(VERSION 3.16)
project(HashReverse)

set(CMAKE_CXX_STANDARD 17)

add_compile_options(-mbmi2)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")

file(GLOB_RECURSE SOURCE_FILES "src/*.cc")

add_executable(HashReverse main.cc ${SOURCE_FILES})
add_executable(test test.cc ${SOURCE_FILES})
add_executable(bench bench.cc ${SOURCE_FILES})

target_link_libraries(HashReverse pthread)
target_link_libraries(test pthread)
target_link_libraries(bench pthread)