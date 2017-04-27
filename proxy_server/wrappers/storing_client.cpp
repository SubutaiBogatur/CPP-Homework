//
// Created by Aleksandr Tukallo on 27.04.17.
//

#define _GLIBCXX_USE_CXX11_ABI 0 //for nice gdb debugging

#include <unistd.h>
#include <cstring>
#include "storing_client.h"
#include "../utils/util_functions.h"

storing_client::storing_buffer::~storing_buffer()
{
    delete[] buffer;
}

void storing_client::storing_buffer::shl(size_t val)
{
    filled -= val;
    //here memmove is used instead of memcpy, because memcpy has UB, when src and dest
    //  overlap and memmove doesn't
    std::memmove(buffer, buffer + val, filled);
}

storing_client::storing_client(int fd, size_t buffer_size) : file_descriptor(fd), st_buffer(storing_buffer(buffer_size))
{
    utils::ensure("Got client with fd: " + std::to_string(fd) + "\n");
}

size_t storing_client::get_filled()
{
    return st_buffer.filled;
}

bool storing_client::is_buffer_empty()
{
    return this->get_filled() == 0;
}

bool storing_client::is_nasty(size_t buffer_size)
{
    return this->st_buffer.buffer_size - this->get_filled() <= buffer_size;
}

ssize_t storing_client::read_from_client(size_t count)
{
    return abstract_read(count);
}

void storing_client::write_to_client()
{
    abstract_write();
}

void storing_client::test_write_to_client(int n)
{
    if (std::rand() % n == 0)
    {
        utils::ensure(0, utils::is_zero,
                      "Attempt to write to client " + std::to_string(get_fd()) + " failed. Buffer size is " +
                      std::to_string(st_buffer.buffer_size) + " with " + std::to_string(st_buffer.filled) +
                      " filled\n");
        return;
    }
    write_to_client();
}

ssize_t storing_client::abstract_read(size_t count)
{
    ssize_t r = read(get_fd(), (void *) (this->st_buffer.buffer + this->get_filled()),
                     count); //our invariant is that there is always such free size in storing buffer
    this->st_buffer.filled += r;
    utils::ensure(r, utils::is_not_negative,
                  r != 0 ? std::to_string(r) + " bytes read from the client and put in storing buffer\n" : "");
    return r;
}

void storing_client::abstract_write()
{
    ssize_t w = write(get_fd(), (void *) this->st_buffer.buffer, this->get_filled());
    utils::ensure(w, utils::is_not_negative, "");
    this->st_buffer.shl((size_t) w); //casting is ok, 'cause non negative

    utils::ensure(std::to_string(w) + " bytes written, " + std::to_string(this->get_filled())
                  + " left in buffer of client " + std::to_string(get_fd()) + "\n");
}
