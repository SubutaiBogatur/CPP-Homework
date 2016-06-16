cmake_minimum_required(VERSION 3.5)

project(LongArithmeticsGCC)

include_directories(${BIGINT_SOURCE_DIR})

add_executable(big_integer_testing
        big_integer_testing.cpp
        big_integer.h
        big_integer.cpp
        gtest/gtest-all.cc
        gtest/gtest.h
        gtest/gtest_main.cc)

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -pedantic")
endif()

target_link_libraries(big_integer_testing -lgmp -lgmpxx -lpthread)
