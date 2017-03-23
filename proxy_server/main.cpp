#include <iostream>
#include <netinet/in.h>

void start_echo_server(uint16_t port);

int main()
{
    std::cout << "Main started!" << std::endl;
    start_echo_server(8666);
    return 0;
}

