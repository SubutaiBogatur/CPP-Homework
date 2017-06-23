project(trampoline_hw)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS -g)
add_executable(${PROJECT_NAME} main.cpp trampoline.h)

