project(proxy_server)
cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS)
add_executable("echo_server"
        main.cpp
        echo_server.cpp echo_server.h
        timeouts/deadline_container.cpp timeouts/deadline_container.h
        timeouts/deadline_wrapper.h
#        timeouts/deadline_container_tester.cpp timeouts/deadline_container_tester.h
        utils/util_functions.cpp utils/util_functions.h utils/server_exception.h
        wrappers/epoll_wrapper.cpp wrappers/epoll_wrapper.h
        wrappers/file_descriptor.cpp wrappers/file_descriptor.h
        wrappers/storing_client.cpp wrappers/storing_client.h
        wrappers/deadline_client.cpp wrappers/deadline_client.h
        wrappers/echo_client.cpp wrappers/echo_client.h)

