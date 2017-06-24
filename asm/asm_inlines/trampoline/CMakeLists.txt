project(trampoline)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS -g)
#set(CMAKE_CXX_FLAGS -Wall) # never set the flag
add_executable(${PROJECT_NAME} main.cpp trampoline.h argument_types.h allocator.h allocator.cpp)

