cmake_minimum_required(VERSION 3.5)
project(Lru_cache)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -gdwarf")

set(SOURCE_FILES main.cpp lru-cache.cpp lru-cache.h)
add_executable(Lru_cache ${SOURCE_FILES})