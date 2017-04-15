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

echo_server::echo_server(uint16_t portt)
{
    this->port = portt;
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
                all_clients.erase(lazy_client->get_fd());
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
                                      deadline_wrapper(default_timeout,
                                                       default_timeout + std::time(nullptr), client)));
                all_clients.insert({client->get_fd(), client});
            } else if (res.second[i].data.fd == epoll_.get_signal_fd())
            {
                utils::ensure(0, utils::is_zero, "Server is closing because of a signal.\n");
                throw server_exception("Caught signal"); //exception calls all the destructors, nice
            } else if ((res.second[i].events & EPOLLIN) != 0)
            {
                // if client is ready to write data to us
                client_ptr client = all_clients.at(res.second[i].data.fd);

                if (client->is_nasty(default_buffer_size) || client->read_from_client(default_buffer_size) == 0)
                {
                    utils::ensure(0, utils::is_zero,
                                  client->is_nasty(default_buffer_size) ? "Nasty client " +
                                                                          std::to_string(client->get_fd()) +
                                                                          " soon will be removed\n"
                                                                        : "Client " + std::to_string(client->get_fd()) +
                                                                          " disconnected from the server\n");
                    epoll_.remove_client(client); //stop listening
                    dc.remove(client->get_it());
                    all_clients.erase(client->get_fd());
                    continue;
                }

                //if successfully read from client, add it to write to it
                res.second[res.first].events = EPOLLOUT; //write can be done
                res.second[res.first++].data.fd = client->get_fd();
                client->set_it(dc.update(client->get_it(), time(0) + default_timeout));
            } else if ((res.second[i].events & EPOLLOUT) != 0)
            {
                //else client has woken up and is ready to read something from our storing buffer
                client_ptr client = all_clients.at(res.second[i].data.fd);
//                client->write_to_client();
                client->test_write_to_client(2);
                client->set_it(dc.update(client->get_it(), time(0) + default_timeout));
            }
        }
    }
}

