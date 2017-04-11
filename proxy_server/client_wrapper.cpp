//
// Created by Aleksandr Tukallo on 11.04.17.
//

#include "client_wrapper.h"
#include "utils/util_functions.h"
#include <unistd.h>
#include <cstring>
#include <sys/socket.h>

//todo is such implementation ok, or some cyclic structure should be implemented here?
void client_wrapper::storing_buffer::shl(size_t val)
{
    filled -= val;
    //here memmove is used instead of memcpy, because memcpy has UB, when src and dest
    //  overlap and memmove doesn't
    std::memmove(buffer, buffer + val, filled);
}

client_wrapper::storing_buffer::~storing_buffer()
{
    delete[] buffer;
}

client_wrapper::client_wrapper(const int server_fd, const size_t buffer_size) : st_buffer(storing_buffer(buffer_size))
{
    fd = accept(server_fd, nullptr, nullptr);

    //if ensure throws an exception, everything is ok, 'cause if it throws it, resource was not
    //  allocated and hence should not be closed in destructor
    utils::ensure(fd, utils::is_not_negative,
                  "Got client with fd: " + std::to_string(fd) + "\n");
}

client_wrapper::~client_wrapper()
{
    utils::ensure(close(fd), utils::is_zero,
                  "Client " + std::to_string(fd) + " was closed\n");
}

int client_wrapper::get_fd()
{
    return fd;
}

list_it client_wrapper::get_it()
{
    return it;
}


//void client_wrapper::init_it(std::list<timeout_wrapper>& queue, std::shared_ptr<client_wrapper> ptr_to_this)
//{
//    queue.push_back(timeout_wrapper(ptr_to_this));
//    it = --queue.end();
//}


size_t client_wrapper::get_filled()
{
    return st_buffer.filled;
}

bool client_wrapper::is_buffer_empty()
{
    return this->get_filled() == 0;
}

bool client_wrapper::is_nasty(size_t buffer_size)
{
    return this->st_buffer.buffer_size - this->get_filled() < buffer_size;
}

//void client_wrapper::buffer_shl(size_t v)
//{
//    this->st_buffer.shl(v);
//}
//
//client_wrapper::operator std::string()
//{
//    return std::to_string(fd);
//}
//
//void client_wrapper::update_queue(std::list<timeout_wrapper>& queue, std::shared_ptr<client_wrapper> ptr_to_this)
//{
//    queue.erase(it);
//    queue.push_back(timeout_wrapper(ptr_to_this));
//    it = --queue.end();
//}
//
//int client_wrapper::read_cl()
//{
//    int r = (int) read(fd, (void *) (this->st_buffer.buffer + this->get_filled()),
//                       BUFFER_SIZE); //our invariant is that there is always such free size in storing buffer
//    this->st_buffer.filled += r;
//    ensure(r, is_not_negative,
//           r != 0 ? std::to_string(r) + " bytes read from the client and put in storing buffer\n" : "");
//
//    return r;
//}
//
//void client_wrapper::write_cl()
//{
//    int w = (int) write(fd, (void *) this->st_buffer.buffer, this->get_filled());
//    this->buffer_shl(w);
//    ensure(std::to_string(w) + " bytes written, " + std::to_string(this->get_filled()) + " left in buffer\n");
//}
//


