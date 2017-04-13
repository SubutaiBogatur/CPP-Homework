//
// Created by Aleksandr Tukallo on 23.03.17.
//

#include "utils/util_functions.h"
#include "echo_server.h"
#include "timeouts/deadline_container.h"
#include "wrappers/client_wrapper.h"
#include "utils/server_exception.h"
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define _GLIBCXX_USE_CXX11_ABI 0

echo_server::echo_server() : echo_server(default_port)
{}

echo_server::echo_server(uint16_t port)
{
    this->port = port;
    //create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    utils::ensure(server_fd, utils::is_not_negative, "Server fd created " + std::to_string(server_fd) + "\n");

    //todo mb we can encounter problem here, if server opened, but failed on bind and as a result fd not closed
    //  because exception is thrown from constructor
    //todo mb also problem if creating epoll with constructor, it fails, what happens here then?
    //bind socket to port
    sockaddr_in addr; //struct describing internet address
    addr.sin_family = AF_INET; //using ipv4
    addr.sin_port = htons(port); //select port
    addr.sin_addr.s_addr = htonl(INADDR_ANY); //listen to all ips
    utils::ensure(bind(server_fd, (sockaddr *) &addr, sizeof(sockaddr)),
                  utils::is_zero, "Binded to port " + std::to_string(port) + "\n");

}

echo_server::~echo_server()
{
    utils::ensure(close(server_fd), utils::is_zero,
                  "Server fd closed: " + std::to_string(server_fd) + "\n");
}

//todo mb smhw custom max clients if needed
void echo_server::start_listening()
{
    utils::ensure(listen(server_fd, default_max_clients), utils::is_zero,
                  "Socket " + std::to_string(server_fd) + " is in listening state\n");
}

void echo_server::start()
{
    start_listening();
    epoll_.add_server(server_fd); //server_fd listening for EPOLLIN
    epoll_.add_signal_handling();

    deadline_container dc;

    while (true)
    {
        std::pair<int, epoll_event *> res = epoll_.start_sleeping(
                dc.is_empty() ? -1 : static_cast<int>(dc.get_min()->deadline - std::time(nullptr)));

        if (res.first == 0)
        {
            //timeout exceeded
            time_t expired_deadline = dc.get_min()->deadline;

            //todo is it ok to delete all the clients in such a manner?
            while (!dc.is_empty() && dc.get_min()->deadline == expired_deadline)
            {
                client_ptr lazy_client = dc.get_min()->client;
                utils::ensure(0, utils::is_zero,
                              "Lazy client " + std::to_string(lazy_client->get_fd()) + " soon removed from epoll\n");

                epoll_.remove_client(lazy_client); //stop listening
                dc.remove(lazy_client->get_it());
            }
        }

        //lets process all events
        for (int i = 0; i < res.first; i++)
        {
            //if event has happened on server
            if (res.second[i].data.fd == server_fd)
            {
                //for every new client memory is allocated
                client_ptr client = std::make_shared<client_wrapper>(server_fd, this->default_client_buffer_size);
                epoll_.add_client(client);
                client->set_it(dc.add(default_timeout,
                                      deadline_wrapper(default_timeout, default_timeout + std::time(nullptr), client)));
            } else if (res.second[i].data.fd == epoll_.get_signal_fd())
            {
                utils::ensure(0, utils::is_zero, "Server is closing because of a signal.\n");
                throw server_exception("Caught signal"); //exception calls all the destructors, nice
            } //else if ((events[i].events & EPOLLIN) != 0)
//            {
//                // if client is ready to write data to us
//                client_wrapper *client = map.at(events[i].data.fd);
//                if (read_to_storing_buffer(epoll_w, *client))
//                {
//                    //add write to this fd in the queue
//                    events[num].events = EPOLLOUT; //write can be done
//                    events[num++].data.fd = client->get_fd();
//                }
//            } else if ((events[i].events & EPOLLOUT) != 0)
//            {
//                //else client has woken up and is ready to read something from our storing buffer
//                write_to_client(*map.at(events[i].data.fd));
//            }
        }
    }
}

