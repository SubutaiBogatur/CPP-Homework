project(proxy_server)
cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS -g)
add_executable(${PROJECT_NAME}
        main.cpp
        echo_server.cpp echo_server.h
        timeouts/deadline_container.cpp timeouts/deadline_container.h
        timeouts/deadline_wrapper.h
#        timeouts/deadline_container_tester.cpp timeouts/deadline_container_tester.h
        utils/util_functions.cpp utils/util_functions.h utils/server_exception.h
        wrappers/epoll_wrapper.cpp wrappers/epoll_wrapper.h
        wrappers/client_wrapper.cpp wrappers/client_wrapper.h)

