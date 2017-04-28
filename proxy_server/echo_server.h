//
// Created by Aleksandr Tukallo on 12.04.17.
//

#ifndef PROXY_SERVER_ECHO_SERVER_H_H
#define PROXY_SERVER_ECHO_SERVER_H_H

#include "wrappers/epoll_wrapper.h"
#include "wrappers/echo_client.h"
#include <cstdint>
#include <map>

/**
 * @brief This is the main class for starting and managing echo server
 */
struct echo_server
{
private:
    static const uint16_t default_port = 8667;
    static const uint16_t default_max_clients = 16;
    const size_t default_client_buffer_size = 64; //todo why can't be static?
    const size_t default_buffer_size = 16;
    const int default_timeout = 10; //secs

    uint16_t port;
    epoll_wrapper epoll_;
    file_descriptor server_fd;


    void start_listening();

public:
    /**
     * Constructor, where all fields are set as default
     */
    echo_server();

    /**
     * Constructor creates \c server_fd and binds it to given \c port
     */
    echo_server(uint16_t port);

    /**
     * Destructor closes \c server_fd
     */
    ~echo_server();

    echo_server(echo_server const&) = delete;
    echo_server& operator=(echo_server const&) = delete;

    /**
     * Method starts listening to events on a server and in free time waits in \c epoll_wait, not
     * in busy wait
     */
    void start();
};

#endif //PROXY_SERVER_ECHO_SERVER_H_H
