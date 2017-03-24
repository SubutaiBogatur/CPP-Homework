//
// Created by Aleksandr Tukallo on 23.03.17.
//

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <unistd.h>

#include <errno.h>
#include <assert.h>
#include <iostream>
#include <string.h>

bool logging_is_enabled = true;

#define LOG(x) do { \
  if (logging_is_enabled) { std::cerr << x; } \
} while (0)

//in such chunks reading and writing is done
#define BUFFER_SIZE 1024
#define MAX_NUMBER_CLIENTS 128
#define MAX_EVENTS 1024

int create_socket()
{
    //server socket
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket < 0)
    {
        LOG("Error is" + std::string(strerror(errno)) + ". ");
        assert (false);
    }
    LOG(std::string("Socket fd created " + std::to_string(socket_fd) + "\n"));
    return socket_fd;
}

void bind_socket(int socket_fd, uint16_t port)
{
    //bind socket to port and addresses
    sockaddr_in addr; //struct decribing internet address
    addr.sin_family = AF_INET; //using ipv4
    addr.sin_port = htons(port); //select port
    addr.sin_addr.s_addr = htonl(INADDR_ANY); //listen to all ips
    int bind_res = bind(socket_fd, (sockaddr *)&addr, sizeof(sockaddr));
    if (bind_res != 0)
    {
        LOG("Error is" + std::string(strerror(errno)) + ". ");
        assert (false);
    }
    LOG("Binded to port " + std::to_string(port) + "\n");
}

void start_listening(int socket_fd)
{
    int listen_res = listen(socket_fd, MAX_NUMBER_CLIENTS);
    if (listen_res != 0)
    {
        LOG("Error is" + std::string(strerror(errno)) + ". ");
        assert (false);
    }
    LOG("Is in listening state\n");
}

int create_epoll()
{
    int epoll_fd = epoll_create1(0);
    if (epoll_fd < 0)
    {
        LOG("Error is" + std::string(strerror(errno)) + ". ");
        assert (false);
    }
    LOG(std::string("Epoll fd created " + std::to_string(epoll_fd) + "\n"));
    return epoll_fd;
}

void add_listening_to_epoll(int epoll_fd, int socket_fd)
{
    epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = socket_fd;
    int res = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &event);
    if (res != 0)
    {
        LOG("Error is" + std::string(strerror(errno)) + ". ");
        assert (false);
    }
    LOG(std::string("Listening socket " + std::to_string(socket_fd) + " added to epoll " + std::to_string(epoll_fd) + "\n"));
}

int start_sleeping(int epoll_fd, epoll_event* events)
{
    int num = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
    if (num < 0)
    {
        LOG("Error is" + std::string(strerror(errno)) + ". ");
        assert (false);
    }
    LOG(std::string("Epoll has waken up with " + std::to_string(num) + "new events\n"));
    return num;
}

int accept_client(int socket_fd)
{
    int client_fd = accept(socket_fd, 0, 0);
    if (client_fd < 0)
    {
        LOG("Error is" + std::string(strerror(errno)) + ". ");
        assert (false);
    }
    LOG(std::string("Got client with fd: " + std::to_string(client_fd) + "\n"));
    return client_fd;
}

void add_client_to_epoll(int epoll_fd, int client_fd)
{
    epoll_event event;
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = client_fd;
    int res = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event);
    if (res != 0)
    {
        LOG("Error is" + std::string(strerror(errno)) + ". ");
        assert (false);
    }
    LOG("Another client added to epoll\n");
}

void start_echo_server(uint16_t port)
{
    int socket_fd = create_socket();
    bind_socket(socket_fd, port);
    start_listening(socket_fd);

    int epoll_fd = create_epoll(); //flags is zero
    add_listening_to_epoll(epoll_fd, socket_fd);
    epoll_event events[MAX_EVENTS]; //to get after waiting

    char buffer[BUFFER_SIZE];

    while (true)
    {
        //number of events happened, when sleeping
        int num = start_sleeping(epoll_fd, events);
        //lets process all events
        for (int i = 0; i < num; i++)
        {
            //if event has happened on server
            if (events[i].data.fd == socket_fd)
            {
                int client_fd = accept_client(socket_fd);
//                int flags = fcntl(fd, F_GETFL, 0);
//                err = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
                //mb set nonblocking
                add_client_to_epoll(epoll_fd, client_fd);
            }
            else
            {
                char buffer[BUFFER_SIZE]; // buffer we'll use for reading and writing
                //read
                int r = read(events[i].data.fd, (void *) buffer, BUFFER_SIZE);
                assert (r >= 0);

                if (r == 0)
                {
                    LOG(std::string("Client with fd " + std::to_string(events[i].data.fd) + " is closed\n"));
                    //connection closed by client
                    //mb smhw delete client from epoll
                    continue;
                }

                //just way of logging, writing to stderr
                write(2, (void *) buffer, r);

                int w = write(events[i].data.fd, (void *) buffer, r);
                assert (w >= 0);
                assert (w == r); //todo, what if not equals, but not error?
            }
        }
    }
}
