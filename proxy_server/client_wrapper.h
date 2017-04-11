//
// Created by Aleksandr Tukallo on 11.04.17.
//

#ifndef PROXY_SERVER_CLIENT_WRAPPER_H
#define PROXY_SERVER_CLIENT_WRAPPER_H

#define _GLIBCXX_USE_CXX11_ABI 0 //for nice gdb debugging

#include "timeouts/deadline_wrapper.h"
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

//    int get_fd();
//    std::list<timeout_wrapper>::iterator get_it();
//    size_t get_filled();
//    bool is_buffer_empty();
//    void buffer_shl(size_t v);
//    operator std::string();

    //method creates new timeout wrapper pointing to given client_wrapper and pushes it in q, updating it
//    void init_it(std::list<timeout_wrapper>& queue, std::shared_ptr<client_wrapper> ptr_to_this);

    //method reads BUFFER_SIZE bytes to current storing buffer and returns what syscall read returns
//    int read_cl();

    //methdd writes buffer.filled to fd. Then shl on buffer is made
//    void write_cl();

    //let's call client nasty, if it writes much faster, than reads.
    //  As a result there is not much free space in storing buffer left
//    bool is_nasty();
};


#endif //PROXY_SERVER_CLIENT_WRAPPER_H
