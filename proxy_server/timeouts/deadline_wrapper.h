//
// Created by Aleksandr Tukallo on 08.04.17.
//

#ifndef TIMEOUT_WRAPPER_H
#define TIMEOUT_WRAPPER_H

#include <ctime>
#include <memory>

struct client_wrapper;

/**
 * @brief Structure is needed to store upcoming deadlines.
 */
struct deadline_wrapper
{
    typedef std::shared_ptr<client_wrapper> client_ptr;

    int timeout;
    time_t deadline;
    client_ptr client;
//    int client; //todo debug only, fd instead of struct

//    deadline_wrapper(int timeout, time_t deadline, int client)
    deadline_wrapper(int timeout, time_t deadline, client_ptr client)
            : timeout(timeout), deadline(deadline), client(client)
    {}
};

#endif // TIMEOUT_WRAPPER_H