//
// Created by Aleksandr Tukallo on 23.03.17.
//

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

#include <errno.h>
#include <assert.h>

#include <map>
#include <iostream>
#include <string.h>
#include <memory>
#include <utility>

bool logging_is_enabled = true;

#define LOG(x) do { \
  if (logging_is_enabled) { std::cerr << x; } \
} while (0)

//in such chunks reading and writing is done
#define BUFFER_SIZE 16 //for debugging only
#define STORING_BUFFER_SIZE BUFFER_SIZE * 4
#define MAX_NUMBER_CLIENTS 128
#define MAX_EVENTS 1024
#define PORT 8667

struct storing_buffer
{
    //todo probably some cyclic structure should be implemented here
    size_t filled;
    char buffer[STORING_BUFFER_SIZE];

    storing_buffer()
    {
        filled = 0;
    }

    //todo strongly tmp function, which mb can be later optimized:
    //moves all bytes left
    void shl(size_t val)
    {
        //here we have terrible problems with aliasing, thus tmp arr
        char tmp[STORING_BUFFER_SIZE];
        memcpy(tmp, buffer + filled, filled - val);
        memcpy(buffer, tmp, filled - val);
        filled -= val;
    }
};

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

//map stores pointer to all the clients storing buffers
std::map<int, storing_buffer> storing_buffers;

void start_echo_server()
{
    int socket_fd = create_socket();
    bind_socket(socket_fd, PORT);
    start_listening(socket_fd);

    int epoll_fd = create_epoll();
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
                //setnonblocking
                fcntl(client_fd, F_SETFL, fcntl(client_fd, F_GETFD, 0)| O_NONBLOCK);

                epoll_event event;
                event.events = EPOLLIN | EPOLLOUT | EPOLLET;
                event.data.fd = client_fd;
                int res = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event);
                if (res != 0)
                {
                    LOG("Error is" + std::string(strerror(errno)) + ". ");
                    assert (false);
                }
                LOG("Another client added to epoll\n");

                storing_buffers.insert(std::pair<int, storing_buffer>(client_fd, storing_buffer()));

            }
            else if((events[i].events & EPOLLIN) != 0)
            {
                int client_fd = events[i].data.fd;
                //if client is ready to read from
                int r = read(client_fd, (void *) buffer, BUFFER_SIZE);
                assert (r >= 0);

                if (r == 0)
                {
                    LOG(std::string("Client with fd " + std::to_string(client_fd) + " is closed\n"));
                    //connection closed by client
                    storing_buffers.erase(client_fd); //delete from map
                    continue;
                }

                if (r == 2)
                {
                    //todo only for debugging:
                    //  empty string means ready to write to client
                    events[num].events = 4;//write can be done
                    events[num++].data.fd = client_fd;
                    continue;
                }

                //there is no while not to get busy wait, write is nonblocking
                int strongly_tmp = r / 2; //we decide not to write full information first time to test buffers
                int w = write(client_fd, (void *) buffer, strongly_tmp);

                if (w != r)
                {
                    //todo is ok?, what't the diff between -1 and 0 returned values?
                    //todo check if storing buffer is not overflowing
                    w = w == -1 ? 0 : w;

                    int left_to_write = r - w;
                    memcpy(storing_buffers.at(client_fd).buffer + storing_buffers.at(client_fd).filled,
                           buffer + w, left_to_write); //copy unwritten to data to storing buffer
                    storing_buffers.at(client_fd).filled += left_to_write;
                }
            }
            else
            {
                //else client has woken up and is ready to read something from our storing buffer
                int client_fd = events[i].data.fd;
                if (storing_buffers.at(client_fd).filled == 0)
                {
                    //there is nothing more to write
                    continue;
                }
                int w = write(client_fd,
                              (void *) storing_buffers.at(client_fd).buffer,
                              storing_buffers.at(client_fd).filled);
                storing_buffers.at(client_fd).shl(w);
            }
        }
    }
}
