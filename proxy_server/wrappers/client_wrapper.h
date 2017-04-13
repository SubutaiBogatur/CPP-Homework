//
// Created by Aleksandr Tukallo on 11.04.17.
//

#ifndef PROXY_SERVER_CLIENT_WRAPPER_H
#define PROXY_SERVER_CLIENT_WRAPPER_H

#define _GLIBCXX_USE_CXX11_ABI 0 //for nice gdb debugging

#include "../timeouts/deadline_wrapper.h"
#include <list>

/**
 * Structure should be used to store every client of a server. It provides comfortable methods to
 * do operations with clients
 */
struct client_wrapper
{
public:
    typedef std::list<deadline_wrapper>::iterator list_it;

private:
    struct storing_buffer
    {
        size_t filled;
        const size_t buffer_size;
        char *const buffer;

        storing_buffer(const size_t buffer_size) : filled(0), buffer_size(buffer_size), buffer(new char[buffer_size])
        {}

        ~storing_buffer();

        /**
         * Method moves bytes in \c buffer \c val bytes left, \c filled is also reduced by \c val.
         * The method should be usually used, when \c write from storing buffer happens. Written
         * bytes shouldn't be stored anymore
         */
        void shl(size_t val);
    };

    int fd;
    storing_buffer st_buffer;
    list_it it; //position in queue of timeouts

public:

    /**
     * Constructor calls \c accept syscall on given server. It was decided to make \c accept call in
     * constructor to follow RAII idiom: constructor opens fd (ie resource), destructor closes it
     * @param server_fd is server that has clients in pending queue
     * @param buffer_size is size of buffer, that will be used to store data, that is written by client,
     */
    client_wrapper(int server_fd, size_t buffer_size);

    /**
     * Destructor closes the \c fd with respective syscall
     */
    ~client_wrapper();

    /**
     * Method returns \c fd of this client
     */
    int get_fd();

    /**
     * Method returns iterator to node in queue, where deadline for this client is stored
     */
    list_it get_it();

    void set_it(list_it it);

    /**
     * Method returns number of bytes in \c buffer, that are filled with data
     * @return \c st_buffer.filled
     */
    size_t get_filled();

    /**
     * Returns true if \c buffer is empty
     */
    bool is_buffer_empty();

    /**
     * Let's call client nasty, if it writes much faster, than reads.
     * As a result there is not much free space in storing buffer left and
     * probably client should be disconnected, because it abuses our trust
     * @param buffer_size is size of buffer in which all read and write operations are done
     * @return true, if \c this->st_buffer->buffer_size  \c - \c this->get_filled() is less than \c buffer_size;
     */
    bool is_nasty(size_t buffer_size);

//    void buffer_shl(size_t v);
//    operator std::string();

    //method creates new timeout wrapper pointing to given client_wrapper and pushes it in q, updating it
//    void init_it(std::list<timeout_wrapper>& queue, std::shared_ptr<client_wrapper> ptr_to_this);

//  method reads count bytes to current storing buffer and returns what syscall read returns
//  careful: check if not nasty before calling read
    ssize_t read_from_client(size_t count);

    //method writes buffer.filled bytes from buffer to fd. Then shl on buffer is made
    void write_to_client();

    //method doesn't do write with probability 1/n. It is needed to test buffer overflowing
    void test_write_to_client(int n);
};


#endif //PROXY_SERVER_CLIENT_WRAPPER_H
