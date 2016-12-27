cmake_minimum_required(VERSION 3.5)

project(LongArithmeticsGCC)

include_directories(${BIGINT_SOURCE_DIR})

add_executable(big_integer_testing
        big_integer_testing.cpp
        big_integer.h
        big_integer.cpp
        gtest/gtest-all.cc
        gtest/gtest.h
        gtest/gtest_main.cc optimized_vector.cpp optimized_vector.h)

add_executable(main main.cpp big_integer.cpp big_integer.h optimized_vector.cpp optimized_vector.h)

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -std=c++11 -O3")
endif()

target_link_libraries(big_integer_testing -lgmp -lgmpxx -lpthread)
