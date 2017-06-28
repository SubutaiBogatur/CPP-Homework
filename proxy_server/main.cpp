//
// Created by Aleksandr Tukallo on 09.04.17.
//


#include "utils/server_exception.h"
#include <iostream>
#include "echo_server.h"

int main()
{
    try
    {
        echo_server server;
        server.start();
    } catch (server_exception const& e)
    {
        std::cerr << e.what() << std::endl;
    }
}
