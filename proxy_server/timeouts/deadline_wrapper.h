//
// Created by Aleksandr Tukallo on 08.04.17.
//

#ifndef TIMEOUT_WRAPPER_H
#define TIMEOUT_WRAPPER_H

#include <ctime>
#include <memory>

struct file_descriptor;

/**
 * @brief Structure is needed to wrap deadline, timeout and ptr to client in one object,
 * which will be stored in \c deadline_container
 */
struct deadline_wrapper
{
    typedef file_descriptor *fd_ptr;

    int timeout;
    time_t deadline;
    fd_ptr client;

    deadline_wrapper(int timeout, time_t deadline, fd_ptr client)
            : timeout(timeout), deadline(deadline), client(client)
    {}

    deadline_wrapper(int timeout, fd_ptr client)
            : timeout(timeout), deadline(std::time(nullptr) + timeout), client(client)
    {}
};

#endif // TIMEOUT_WRAPPER_H