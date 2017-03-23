//
// Created by Aleksandr Tukallo on 23.03.17.
//

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include <assert.h>
#include <iostream>
#include <string>

bool logging_is_enabled = true;

#define LOG(x) do { \
  if (logging_is_enabled) { std::cerr << x; } \
} while (0)

#define BUFFER_SIZE 1024

void start_echo_server(uint16_t port)
{
    //server socket
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    assert (socket_fd >= 0);
    LOG(std::string("Socket fd created " + std::to_string(socket_fd) + "\n"));

    //bind socket to port and addresses
    sockaddr_in addr; //struct decribing internet address
    addr.sin_family = AF_INET; //using ipv4
    addr.sin_port = htons(port); //select port
    addr.sin_addr.s_addr = htonl(INADDR_ANY); //listen to all ips
    assert (bind(socket_fd, (sockaddr *)&addr, sizeof(sockaddr)) == 0);
    LOG("Binded to port " + std::to_string(port) + "\n");

    int max_number_of_clients = 10;
    assert (listen(socket_fd, max_number_of_clients) == 0); //no more, than 10 clients in queue
    LOG("Is in listening state with max num of clients: " + std::to_string(max_number_of_clients) + "\n");

    //work with new and new clients while is able to
    while(true)
    {
        //try to get new client, if pending connections queue is empty,
        //  wait until someone comes, server is in blocking state
        int client_socket_fd = accept(socket_fd, 0, 0);
        assert (client_socket_fd >= 0);
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

            int w = write(client_socket_fd, (void *) buffer, r);
            assert (w >= 0);
            assert (w == r); //todo, what if not equals, but not error?

        }
        LOG("Work with client is finished\n");
    }

}
