//
// Created by Aleksandr Tukallo on 11.04.17.
//

#ifndef PROXY_SERVER_EPOLL_WRAPPER_H
#define PROXY_SERVER_EPOLL_WRAPPER_H

#include <memory>
#include <sys/epoll.h>

struct client_wrapper;

/**
 * @brief Structure is needed to provide comfortable methods for dealing with epoll instance, also
 * it provides RAII idiom for it
 */
struct epoll_wrapper
{
private:
    static const uint16_t default_max_epoll_events = 16;

    int epoll_fd;
    int signal_fd; //todo probably it should be an optional not to close not existing and also not to add twice
    epoll_event* events;

public:
    /**
     * Method returns \c epoll_fd
     * @return
     */
    int get_fd();

    
    int get_signal_fd();

    /**
     * Constructor creates new epoll instance using syscall. Its \c fd is saved in \c epoll_fd field
     */
    epoll_wrapper();

    /**
     * Destructor closes \c epoll_fd and \c signal_fd
     */
    ~epoll_wrapper();

    /**
     * Method adds listening socket to epoll, listening to \c EPOLLIN events on it (listening for new clients)
     */
    void add_server(int server_fd);

    //method returns number of events occured, when got up
    std::pair<int, epoll_event*> start_sleeping(int timeout);

    /**
     * Method adds client to epoll. Now epoll waits for in, out events on it.
     * Added client is edge triggered (ie same events won't come twice)
     * @param client client to add
     */
    void add_client(std::shared_ptr<client_wrapper> client);

    //method removes client from epoll: events on it are not detected anymore
    void remove_client(std::shared_ptr<client_wrapper> client);

    /**
     * Method adds \c signalfd to epoll. Signalfd is created inside the method and
     * \c SIGTERM and \c SIGINT signals will be catched by that fd. Signalfd is
     * added to epoll and now epoll listens to signals catched by that fd
     *
     * Method should be called only once. //todo optional, what if twice,
     * todo add method with custom signals
     */
    void add_signal_handling();
};


#endif //PROXY_SERVER_EPOLL_WRAPPER_H
