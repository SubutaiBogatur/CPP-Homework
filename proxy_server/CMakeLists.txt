project(proxy_server)
cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS -g)
add_executable(${PROJECT_NAME} echo_server.cpp)

