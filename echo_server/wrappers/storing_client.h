//
// Created by Aleksandr Tukallo on 27.04.17.
//

#ifndef PROXY_SERVER_STORING_CLIENT_H
#define PROXY_SERVER_STORING_CLIENT_H


#include <cstdlib>
#include "file_descriptor.h"

struct storing_client : public file_descriptor
{
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

    storing_buffer st_buffer;

protected:

    ssize_t abstract_read(size_t count);
    void abstract_write();

public:
    storing_client(int fd, size_t buffer_size);

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

    //  method reads count bytes to current storing buffer and returns what syscall read returns
//  careful: check if not nasty before calling read
    virtual ssize_t read_from_client(size_t count);

    //method writes buffer.filled bytes from buffer to fd. Then shl on buffer is made
    virtual void write_to_client();

    //method doesn't do write with probability 1/n. It is needed to test buffer overflowing
    void test_write_to_client(int n);
};


#endif //PROXY_SERVER_STORING_CLIENT_H
