#include <iostream>
#include <netinet/in.h>

void start_echo_server();

int main()
{
    std::cout << "Main started!" << std::endl;
    start_echo_server();
    return 0;
}

