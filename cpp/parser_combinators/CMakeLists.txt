cmake_minimum_required(VERSION 3.0)
project(control_2)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}\
    -Wall\
    -Werror\
")

add_executable(control_2 test.cpp parser_combinators.hpp)
