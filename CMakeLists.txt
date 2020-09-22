cmake_minimum_required(VERSION 3.16)
project(HashReverse)

set(CMAKE_CXX_STANDARD 17)

add_compile_options(-mbmi2)

file(GLOB_RECURSE SOURCE_FILES "src/*.cc")

add_executable(HashReverse main.cpp ${SOURCE_FILES})