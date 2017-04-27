//
// Created by Aleksandr Tukallo on 27.04.17.
//

#ifndef PROXY_SERVER_DEADLINE_CLIENT_H
#define PROXY_SERVER_DEADLINE_CLIENT_H


#include "../timeouts/deadline_container.h"
#include "storing_client.h"

/**
 * This structure knows about deadline container and adds itself to it
 */
struct deadline_client : public storing_client
{
public:
    typedef std::list<deadline_wrapper>::iterator list_it;

private:
    deadline_container& dc;
    list_it it; //position in queue of timeouts
    int timeout; //timeout which is set for this client after every modification

public:
    deadline_client(int fd, size_t buffer_size, deadline_container& dc, int timeout);
    ~deadline_client();

    //methods do the same as in parent, but also update dc
    virtual ssize_t read_from_client(size_t count);
    virtual void write_to_client();
};


#endif //PROXY_SERVER_DEADLINE_CLIENT_H
