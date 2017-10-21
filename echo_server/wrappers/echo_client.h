//
// Created by Aleksandr Tukallo on 27.04.17.
//

#ifndef PROXY_SERVER_ECHO_CLIENT_H
#define PROXY_SERVER_ECHO_CLIENT_H


#include "deadline_client.h"
#include "epoll_wrapper.h"

/**
 * Echo client is just like deadline client, but it also knows about epoll.
 * It just adds extra OOP style, adding to epoll in constructor and unsubscribing in destructor
 */
struct echo_client : public deadline_client
{
private:
    epoll_wrapper& epoll_;

public:
    echo_client(int fd, size_t buffer_size, deadline_container& dc, int timeout, epoll_wrapper& epoll_);
    ~echo_client();
};


#endif //PROXY_SERVER_ECHO_CLIENT_H
