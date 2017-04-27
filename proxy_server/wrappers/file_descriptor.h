//
// Created by Aleksandr Tukallo on 27.04.17.
//

#ifndef PROXY_SERVER_FD_H
#define PROXY_SERVER_FD_H


/**
 * Class is needed for exception safe fd, it is closed in destructor, though not created here
 */
struct file_descriptor
{
private:
    int fd_val;
public:
    int get_fd();
    file_descriptor(int);
    virtual ~file_descriptor();
};


#endif //PROXY_SERVER_FD_H
