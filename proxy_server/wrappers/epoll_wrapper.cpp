//
// Created by Aleksandr Tukallo on 11.04.17.
//

#include "../utils/util_functions.h"

#include "epoll_wrapper.h"
#include "client_wrapper.h"

#include <sys/epoll.h>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/signalfd.h>
#include <cstring>


epoll_wrapper::epoll_wrapper()
{
    epoll_fd = epoll_create1(0);
    utils::ensure(epoll_fd, utils::is_not_negative,
                  "Epoll fd created: " + std::to_string(epoll_fd) + "\n");

    events = new epoll_event[default_max_epoll_events];
}

epoll_wrapper::~epoll_wrapper()
{
    utils::ensure(close(epoll_fd), utils::is_zero,
                  "Epoll fd closed: " + std::to_string(epoll_fd) + "\n");
    utils::ensure(close(signal_fd), utils::is_zero,
                  "Signal fd closed: " + std::to_string(signal_fd) + "\n");

    delete[] events;
}

int epoll_wrapper::get_fd()
{
    return epoll_fd;
}

int epoll_wrapper::get_signal_fd() //todo check smhw if present
{
    return signal_fd;
}

void epoll_wrapper::add_server(int server_fd)
{
    epoll_event event;
    memset(&event, 0, sizeof(epoll_event)); //to avoid valgrind warnings
    event.events = EPOLLIN;
    event.data.fd = server_fd;
    utils::ensure(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event), utils::is_zero,
                  "Listening socket " + std::to_string(server_fd) + " added to epoll " + std::to_string(epoll_fd) +
                  "\n");
}

void epoll_wrapper::add_client(std::shared_ptr<client_wrapper> client)
{
    //set nonblocking
    utils::ensure(fcntl(client->get_fd(), F_SETFL,
                        fcntl(client->get_fd(), F_GETFD, 0) | O_NONBLOCK),
                  utils::is_not_negative, "");

    epoll_event event;
    memset(&event, 0, sizeof(epoll_event));
    event.events = EPOLLIN | EPOLLOUT | EPOLLET;
    event.data.fd = client->get_fd();

    utils::ensure(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client->get_fd(), &event),
                  utils::is_zero,
                  "Client " + std::to_string(client->get_fd()) + " added to epoll " + std::to_string(epoll_fd) + "\n");
}

void epoll_wrapper::add_signal_handling()
{
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGINT);

    utils::ensure(sigprocmask(SIG_BLOCK, &mask, NULL), utils::is_not_negative,
                  "Default handlers for signals disabled\n");

    signal_fd = signalfd(-1, &mask, 0);
    utils::ensure(signal_fd, utils::is_not_negative, "Signal fd " + std::to_string(signal_fd) + " created\n");

    epoll_event event;
    memset(&event, 0, sizeof(epoll_event));
    event.events = EPOLLIN;
    event.data.fd = signal_fd;
    utils::ensure(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, signal_fd, &event),
                  utils::is_zero, "Signalfd added to epoll\n");
}

std::pair<int, epoll_event *> epoll_wrapper::start_sleeping(int timeout)
{
    utils::ensure(0, utils::is_not_negative,
                  "Epoll " + std::to_string(epoll_fd) + " is going to sleep with timeout (secs): " +
                  std::to_string(timeout) + "\n");


    int num = epoll_wait(epoll_fd, events, default_max_epoll_events, timeout * 1000); //mul to get millisecs

    utils::ensure(num, utils::is_not_negative, "Epoll has waken up with " + std::to_string(num) + " new events" +
                                               (num == 0 ? ". Timeout exceeded" : "") + "\n");
    return {num, events};
}

void epoll_wrapper::remove_client(std::shared_ptr<client_wrapper> client)
{
    utils::ensure(epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client->get_fd(), nullptr),
                  utils::is_zero, "Client " + std::to_string(client->get_fd()) + " removed from epoll\n");
}
