//
// Created by Aleksandr Tukallo on 23.03.17.
//

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/signalfd.h>
#include <signal.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

#include <errno.h>

#include <ctime>

#include <map>
#include <list>
#include <string.h>
#include <memory>

#include <iostream>

//in such chunks reading and writing is done
#define BUFFER_SIZE 16 //for debugging only
#define STORING_BUFFER_SIZE BUFFER_SIZE * 4
#define MAX_NUMBER_CLIENTS 128
#define MAX_EVENTS 1024
#define PORT 8667
#define TIMEOUT 10 //value in seconds. If client does nothing in this time, it is disconnected

//bunch of functions for nice and beautiful error reporting
static bool is_not_negative(int a)
{
    return a >= 0;
}

static bool is_zero(int a)
{
    return a == 0;
}

struct echo_exception
{
    const std::string msg;
    echo_exception(std::string msg) : msg(msg)
    {}
};

//method ensures, that val satisfies given condition. If function returns false on val, exception
//  is thrown, else string "on_success" is being printed
static void ensure(int val, std::function<bool(int)> success, std::string on_success)
{
    if (!success(val))
    {
        throw echo_exception("Error is" + std::string(strerror(errno)) + ". ");
    }
    std::cerr << on_success;
}

//just logs the string
static void ensure(std::string on_success)
{
    std::cerr << on_success;
}

struct client_wrapper
{
    struct timeout_wrapper;
private:
    struct storing_buffer
    {
        //todo probably some cyclic structure should be implemented here
        size_t filled;
        char buffer[STORING_BUFFER_SIZE];

        storing_buffer()
        {
            filled = 0;
        }

        //todo mb can be later optimized:
        //moves all bytes left by val bytes
        void shl(size_t val)
        {
            //here we have terrible problems with aliasing, thus tmp arr
            char tmp[STORING_BUFFER_SIZE];
            memcpy(tmp, buffer + val, filled - val);
            memcpy(buffer, tmp, filled - val);
            filled -= val;
        }
    };

    const int fd;
    storing_buffer buffer;
    std::list<timeout_wrapper>::iterator it; //position in queue

public:

    struct timeout_wrapper
    {
        time_t time; //ie time, when some action should happen, POSIX time since 1970
        client_wrapper *const client; //client, with which action is accosiated

        void update_time()
        {
            time = std::time(nullptr) + TIMEOUT;
        }

        timeout_wrapper(client_wrapper& client) : client(&client)
        {
            update_time();
        }
    };

    //constructor puts timeout in queue
    client_wrapper(const int fd, std::list<timeout_wrapper>& queue) : fd(fd)
    {
        queue.push_back(timeout_wrapper(*this));
        it = --queue.end();
    }

    int get_fd()
    {
        return fd;
    }

    size_t& get_filled()
    {
        return buffer.filled;
    }

    const char *get_buffer()
    {
        return buffer.buffer;
    }

    bool is_buffer_empty()
    {
        return this->get_filled() == 0;
    }

    void buffer_shl(size_t v)
    {
        this->buffer.shl(v);
    }

    operator std::string()
    {
        return std::to_string(fd);
    }

    void remove_from_queue(std::list<timeout_wrapper>& queue)
    {
        queue.erase(it);
    }

    //this method is called, when the client has shown some activity
    //  (eg it read data, or wrote it, or was accepted)
    //  The method adds another node to queue at the end, giving
    //  it needed timeout (curtime + TIMEOUT). Old node is obv being deleted
    //invariant: iterator is already pointing to smwh in queue
    void update_queue(std::list<timeout_wrapper>& queue)
    {
        this->remove_from_queue(queue);
        queue.push_back(timeout_wrapper(*this));
        it = --queue.end();
    }

    //method reads BUFFER_SIZE bytes to current storing buffer and returns what syscall read returns
    int read_cl()
    {
        int r = (int) read(fd, (void *) (this->get_buffer() + this->get_filled()),
                           BUFFER_SIZE); //our invariant is that there is always such free size in storing buffer
        this->get_filled() += r;
        ensure(r, is_not_negative,
               r != 0 ? std::to_string(r) + " bytes read from the client and put in storing buffer\n" : "");
        return r;
    }

    void write_cl()
    {
        int w = (int) write(fd, (void *) this->get_buffer(), this->get_filled());
        this->buffer_shl(w);
        ensure(std::to_string(w) + " bytes written, " + std::to_string(this->get_filled()) + " left in buffer\n");
    }

    //let's call client nasty, if it writes much faster, than reads.
    //  As a result there is not much free space in storing buffer left
    bool is_nasty()
    {
        return STORING_BUFFER_SIZE - this->get_filled() < BUFFER_SIZE;
    }
};

struct epoll_wrapper
{
private:
    const int fd;
    int signal_fd;

public:
    int get_fd()
    {
        return fd;
    }

    int get_signal_fd()
    {
        return signal_fd;
    }

    //constructor creates new epoll fd using syscall
    epoll_wrapper() : fd(epoll_create1(0))
    {
        ensure(fd, is_not_negative, "Epoll fd created: " + std::to_string(fd) + "\n");
    }

    void add_listening(int server_fd)
    {
        epoll_event event;
        event.events = EPOLLIN;
        event.data.fd = server_fd;
        int res = epoll_ctl(fd, EPOLL_CTL_ADD, server_fd, &event);
        ensure(res, is_zero,
               "Listening socket " + std::to_string(server_fd) + " added to epoll " + std::to_string(fd) + "\n");
    }

    //method returns number of events occured, when got up
    int start_sleeping(epoll_event *events, int timeout)
    {
        ensure("Epoll " + std::to_string(fd) + " is going to sleep with timeout (secs): " + std::to_string(timeout) +
               "\n");
        int num = epoll_wait(fd, events, MAX_EVENTS, timeout * 1000); //mul to get millisecs
        ensure(num, is_not_negative, "Epoll has waken up with " + std::to_string(num) + " new events" +
                                     (num == 0 ? ". Timeout exceeded" : "") + "\n");
        return num;
    }

    //method adds client to epoll. Now epoll waits for in, out events on it.
    //  Added client is edge triggered (ie same events won't come twice)
    void add_client(client_wrapper client)
    {
        //setnonblocking
        int res = fcntl(client.get_fd(), F_SETFL, fcntl(client.get_fd(), F_GETFD, 0) | O_NONBLOCK);
        ensure(res, is_not_negative, "");

        epoll_event event;
        event.events = EPOLLIN | EPOLLOUT | EPOLLET;
        event.data.fd = client.get_fd();
        res = epoll_ctl(fd, EPOLL_CTL_ADD, client.get_fd(), &event);
        ensure(res, is_zero, "Client " + (std::string) client + " added to epoll " + std::to_string(fd) + "\n");
    }

    void remove_client(client_wrapper client)
    {
        epoll_ctl(fd, EPOLL_CTL_DEL, client.get_fd(), NULL);
    }

    void add_signal_handling()
    {
        sigset_t mask;
        sigemptyset(&mask);
        sigaddset(&mask, SIGTERM);
        sigaddset(&mask, SIGINT);

        ensure(sigprocmask(SIG_BLOCK, &mask, NULL), is_not_negative,
               "Default handlers for signals disabled\n");

        signal_fd = signalfd(-1, &mask, 0);
        ensure(signal_fd, is_not_negative, "");

        epoll_event event;
        event.events = EPOLLIN;
        event.data.fd = signal_fd;
        epoll_ctl(fd, EPOLL_CTL_ADD, signal_fd, &event);
    }
};

void log_list(std::list<client_wrapper::timeout_wrapper> queue)
{
    std::cerr << (queue.empty() ? "No events in queue" : "");
    for (auto it = queue.begin(); it != queue.end(); it++)
    {
        std::cerr << "fd: " << it->client->get_fd() << " time: " << it->time << " |";
    }
    std::cerr << std::endl;
}

struct echo_server
{
    //map allows to get client_wrapper quickly knowing only client fd
    //  it also allows to check if given client is already present
    std::map<int, client_wrapper *> map;
    std::list<client_wrapper::timeout_wrapper> upcoming_events;

    ~echo_server()
    {
        for (auto it = map.begin(); it != map.end(); it++)
        {
            delete it->second;
        }
    }

    echo_server()
    {}

    int create_socket()
    {
        //server socket
        int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
        ensure(socket_fd, is_not_negative, "Socket fd created " + std::to_string(socket_fd) + "\n");
        return socket_fd;
    }

    void bind_socket(int socket_fd, uint16_t port)
    {
        //bind socket to port and addresses
        sockaddr_in addr; //struct decribing internet address
        addr.sin_family = AF_INET; //using ipv4
        addr.sin_port = htons(port); //select port
        addr.sin_addr.s_addr = htonl(INADDR_ANY); //listen to all ips
        int bind_res = bind(socket_fd, (sockaddr *) &addr, sizeof(sockaddr));
        ensure(bind_res, is_zero, "Binded to port " + std::to_string(port) + "\n");
    }

    void start_listening(int socket_fd)
    {
        int listen_res = listen(socket_fd, MAX_NUMBER_CLIENTS);
        ensure(listen_res, is_zero, "Socket " + std::to_string(socket_fd) + " is in listening state\n");
    }

    int accept_client(int socket_fd)
    {
        int client_fd = accept(socket_fd, 0, 0);
        ensure(client_fd, is_not_negative, "Got client with fd: " + std::to_string(client_fd) + "\n");
        return client_fd;
    }

    //returns true if something was read
    bool read_to_storing_buffer(epoll_wrapper epoll_w, client_wrapper& client)
    {
        ensure("Client " + (std::string) client + " is ready to read from\n");
        int r = client.read_cl();

        if (client.is_nasty()) //nasty nasty
        {
            //close nasty one
            ensure(close(client.get_fd()), is_zero,
                   "Nasty client " + (std::string) client + " closed and soon removed from epoll\n");
        }
        if (r == 0 || client.is_nasty())
        {
            ensure(std::string(r == 0 ? "Client with fd " + (std::string) client + " closed the connection\n" : ""));
            epoll_w.remove_client(client);
            map.erase(map.find(client.get_fd()));
            client.remove_from_queue(upcoming_events);
            return false;
        }

        client.update_queue(upcoming_events);
        return true;
    }

    void write_to_client(client_wrapper& client)
    {
        ensure("Client " + (std::string) client + " is ready to write to\n");
        if (client.is_buffer_empty())
        {
            return;
        }
        client.write_cl();
        client.update_queue(upcoming_events);
    }

    void start_echo_server()
    {
        int server_fd = create_socket();
        bind_socket(server_fd, PORT);
        start_listening(server_fd);

        epoll_wrapper epoll_w;
        epoll_w.add_listening(server_fd); //socket_fd listening for EPOLLIN
        epoll_w.add_signal_handling();

        epoll_event events[MAX_EVENTS]; //to get after waiting

        while (true)
        {
            log_list(upcoming_events);
            int num = epoll_w.start_sleeping(events,
                                             upcoming_events.empty() ?
                                             -1 : upcoming_events.begin()->time - std::time(nullptr));
            if (num == 0)
            {
                //timeout exceeded
                client_wrapper lazy_client = *upcoming_events.begin()->client;
                ensure(close(lazy_client.get_fd()), is_zero,
                       "Lazy client " + (std::string) lazy_client + " closed and soon removed from epoll\n");
                epoll_w.remove_client(lazy_client);
                map.erase(map.find(lazy_client.get_fd()));
                lazy_client.remove_from_queue(upcoming_events);
                continue;
            }

            //lets process all events
            for (int i = 0; i < num; i++)
            {
                //if event has happened on server
                if (events[i].data.fd == server_fd)
                {
                    //for every new client memory is allocated
                    client_wrapper *client = new client_wrapper(accept_client(server_fd), upcoming_events);
                    map.insert(std::pair<int, client_wrapper *>(client->get_fd(), client));
                    epoll_w.add_client(*client);
                } else if (events[i].data.fd == epoll_w.get_signal_fd())
                {
                    ensure("Server is closing because of a signal.\n");
                    //caught signal, removing all signs of out existence
                    for (auto it = map.begin(); it != map.end(); it++)
                    {
                        ensure(close(it->second->get_fd()), is_zero,
                               "Client " + std::to_string(it->second->get_fd()) + " was closed\n");
                    }
                    ensure(close(server_fd), is_zero, "Server was closed\n");
                    exit(0);
                } else if ((events[i].events & EPOLLIN) != 0)
                {
                    // if client is ready to write data to us
                    client_wrapper *client = map.at(events[i].data.fd);
                    if (read_to_storing_buffer(epoll_w, *client))
                    {
                        //add write to this fd in the queue
                        events[num].events = EPOLLOUT; //write can be done
                        events[num++].data.fd = client->get_fd();
                    }
                } else if ((events[i].events & EPOLLOUT) != 0)
                {
                    //else client has woken up and is ready to read something from our storing buffer
                    write_to_client(*map.at(events[i].data.fd));
                }
            }
        }
    }
};


int main()
{
    echo_server echo;
    echo.start_echo_server();

    return 0;
}
