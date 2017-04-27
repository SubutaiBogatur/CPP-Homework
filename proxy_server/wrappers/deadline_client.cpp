//
// Created by Aleksandr Tukallo on 27.04.17.
//

#include "deadline_client.h"

deadline_client::deadline_client(int fd, size_t buffer_size, deadline_container& dc, int timeout)
        : storing_client(fd, buffer_size), dc(dc), timeout(timeout),
          it(dc.add(timeout, deadline_wrapper(timeout, this)))
{
}

ssize_t deadline_client::read_from_client(size_t count)
{
    ssize_t ret = abstract_read(count);
    it = dc.update(it, std::time(nullptr) + timeout);
    return ret;
}

void deadline_client::write_to_client()
{
    abstract_write();
    it = dc.update(it, std::time(nullptr) + timeout);
}

deadline_client::~deadline_client()
{
    dc.remove(it);
}
