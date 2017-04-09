//
// Created by Aleksandr Tukallo on 08.04.17.
//

#ifndef TIMEOUT_WRAPPER_H
#define TIMEOUT_WRAPPER_H

#include <ctime>
#include <memory>

struct client_wrapper;

/**
 * @brief Structure is needed to store upcoming deadliness
 */
struct deadline_wrapper
{
    int timeout;
    time_t deadline;
//    std::shared_ptr<client_wrapper> client;
    int client; //todo debug only, fd instead of struct

    deadline_wrapper(int timeout, time_t deadline, int client)
            : timeout(timeout), deadline(deadline), client(client)
    {}
};

#endif // TIMEOUT_WRAPPER_H