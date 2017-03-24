//
// Created by Aleksandr Tukallo on 23.03.17.
//

#include <sys/types.h>
#include <sys/socket.h>
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

int create_socket()
{
    //server socket
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    LOG("Last error is" + std::string(strerror(errno)) + ". ");
    assert (socket_fd >= 0); //hack to see what's the error
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
    LOG("Last error is" + std::string(strerror(errno)) + ". ");
    assert (bind_res == 0);
    LOG("Binded to port " + std::to_string(port) + "\n");
}

void start_listening(int socket_fd)
{
    int max_number_of_clients = 10;
    int listen_res = listen(socket_fd, max_number_of_clients);
    LOG("Last error is" + std::string(strerror(errno)) + ". ");
    assert (listen_res == 0); //no more, than 10 clients in queue
    LOG("Is in listening state with max num of clients: " + std::to_string(max_number_of_clients) + "\n");
}

void start_echo_server(uint16_t port)
{
    int socket_fd = create_socket();
    bind_socket(socket_fd, port);
    start_listening(socket_fd);

    //work with new and new clients while is able to
    while(true)
    {
        //try to get new client, if pending connections queue is empty,
        //  wait until someone comes, server is in blocking state
        int client_socket_fd = accept(socket_fd, 0, 0);
        assert ((strerror(errno), client_socket_fd >= 0));
        LOG("Got client with socket: " + std::to_string(client_socket_fd) + "\n");

        char buffer[BUFFER_SIZE]; // buffer we'll use for reading and writing
        while(true)
        {
            //read
            int r = read(client_socket_fd, (void *) buffer, BUFFER_SIZE);
            assert (r >= 0);
            if(r == 0)
            {
                //all data is read from the client
                break;
            }

            //just way of logging, writing to stderr
            write(2, (void *) buffer, r);

            int w = write(client_socket_fd, (void *) buffer, r);
            assert (w >= 0);
            assert (w == r); //todo, what if not equals, but not error?

        }
        LOG("Work with client is finished\n");
    }

}
