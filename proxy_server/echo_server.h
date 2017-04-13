//
// Created by Aleksandr Tukallo on 12.04.17.
//

#ifndef PROXY_SERVER_ECHO_SERVER_H_H
#define PROXY_SERVER_ECHO_SERVER_H_H

#include "wrappers/epoll_wrapper.h"
#include <cstdint>
#include <map>

struct echo_server
{
public:
    typedef std::shared_ptr<client_wrapper> client_ptr;

private:
    static const uint16_t default_port = 8667;
    static const uint16_t default_max_clients = 16;
    const size_t default_client_buffer_size = 64; //todo why can't static?
    const size_t default_buffer_size = 16;
    static const int default_timeout = 10; //secs

    uint16_t port;
    epoll_wrapper epoll_;
    int server_fd;

    std::map<int, client_ptr> all_clients; //map is needed to learn from epoll event what client is active

    void start_listening();

public:
    echo_server();

    /**
     * Constructor creates \c server_fd and binds it to port
     * @param port
     */
    echo_server(uint16_t port);

    /**
     * Destructor closes \c server_fd
     */
    ~echo_server();

    echo_server(echo_server const&) = delete;
    echo_server& operator=(echo_server const&) = delete;

    void start();
};

#endif //PROXY_SERVER_ECHO_SERVER_H_H
