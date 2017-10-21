//
// Created by Aleksandr Tukallo on 23.03.17.
//

#include "utils/util_functions.h"
#include "echo_server.h"
#include "utils/server_exception.h"

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define _GLIBCXX_USE_CXX11_ABI 0

echo_server::echo_server() : echo_server(default_port)
{}

echo_server::echo_server(uint16_t portt) : server_fd(file_descriptor(socket(AF_INET, SOCK_STREAM, 0)))
{
    //in the beginning epoll constructor is called with empty argument list
    utils::ensure(server_fd.get_fd(), utils::is_not_negative,
                  "Server fd created " + std::to_string(server_fd.get_fd()) + "\n");

    this->port = portt;

    //bind socket to port
    sockaddr_in addr; //struct describing internet address
    addr.sin_family = AF_INET; //using ipv4
    addr.sin_port = htons(port); //select port
    addr.sin_addr.s_addr = htonl(INADDR_ANY); //listen to all ips
    utils::ensure(bind(server_fd.get_fd(), (sockaddr *) &addr, sizeof(sockaddr)),
                  utils::is_zero, "Binded to port " + std::to_string(port) + "\n");
}

echo_server::~echo_server()
{
    utils::ensure("Server destructor called\n");
}

//todo mb smhw custom max clients if needed
void echo_server::start_listening()
{
    utils::ensure(listen(server_fd.get_fd(), default_max_clients), utils::is_zero,
                  "Socket " + std::to_string(server_fd.get_fd()) + " is in listening state\n");
}

void echo_server::start()
{
    start_listening();
    epoll_.add_server(&server_fd); //server_fd listening for EPOLLIN
    epoll_.add_signal_handling();

    deadline_container dc;

    //This map is the only dogma, the source of truth
    std::map<int, std::shared_ptr<echo_client>> all_clients; //map is needed to learn from epoll_event what client is active

    while (true)
    {
        //res contains pair: (number of events, array of events)
        std::pair<int, epoll_event *> res = epoll_.start_sleeping(
                dc.is_empty() ? -1 : static_cast<int>(dc.get_min()->deadline - std::time(nullptr)));

        if (res.first == 0)
        {
            //timeout exceeded
            time_t expired_deadline = dc.get_min()->deadline;

            //todo is it ok to delete all the clients in such a manner?
            while (!dc.is_empty() && dc.get_min()->deadline == expired_deadline)
            {
                file_descriptor *lazy_client = dc.get_min()->client;
                utils::ensure("Lazy client " + std::to_string(lazy_client->get_fd()) + " soon removed from server\n");
                all_clients.erase(lazy_client->get_fd()); //huge improvement added here. For advanced exception safety
                //  removing client from the only place
            }
        }

        //lets process all events
        for (int i = 0; i < res.first; i++)
        {
            //if event has happened on server
            if (res.second[i].data.fd == server_fd.get_fd())
            {
                //for every new client memory is allocated
                int client_fd = accept(server_fd.get_fd(), nullptr, nullptr);
                utils::ensure(client_fd, utils::is_not_negative,
                              "New client accepted: " + std::to_string(client_fd) + " and soon wrapped\n");
                all_clients.insert(
                        {client_fd, std::make_shared<echo_client>(
                                client_fd, default_client_buffer_size, dc, default_timeout, epoll_)});
            } else if (res.second[i].data.fd == epoll_.get_signal_fd())
            {
                utils::ensure("Server is closing because of a signal.\n");
                throw server_exception("Caught signal"); //exception calls all the destructors, nice
            } else if (res.second[i].events & EPOLLIN)
            {
                // if client is ready to write data to us
                std::shared_ptr<echo_client> client = all_clients[res.second[i].data.fd];

                if (client->is_nasty(default_buffer_size) || client->read_from_client(default_buffer_size) == 0)
                {
                    utils::ensure(client->is_nasty(default_buffer_size)
                                  ? "Nasty client " + std::to_string(client->get_fd()) + " soon will be removed\n"
                                  : "Client " + std::to_string(client->get_fd()) + " disconnected from the server\n");

                    //when erasing from map, destructor is called and client is also removed from dc and epoll_
                    all_clients.erase(client->get_fd());
                    continue;
                }

                //if successfully read from client, add it to write to it
                //todo mb check if size of buffer is not exceeded
                res.second[res.first].events = EPOLLOUT; //write can be done
                res.second[res.first++].data.fd = client->get_fd();
            } else if ((res.second[i].events & EPOLLOUT) != 0)
            {
                //else client has woken up and is ready to read something from our storing buffer
                std::shared_ptr<echo_client> client = all_clients[res.second[i].data.fd];
                client->write_to_client();
//                client->test_write_to_client(2);
            }
        }
    }
}

