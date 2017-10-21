//
// Created by Aleksandr Tukallo on 27.04.17.
//

#include "echo_client.h"

echo_client::echo_client(int fd, size_t buffer_size, deadline_container& dc, int timeout, epoll_wrapper& epoll_)
        : deadline_client(fd, buffer_size, dc, timeout), epoll_(epoll_)
{
    epoll_.add_client(this);
}

echo_client::~echo_client()
{
    epoll_.remove_client(this);
}
