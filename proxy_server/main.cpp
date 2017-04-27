//
// Created by Aleksandr Tukallo on 09.04.17.
//


#include "utils/server_exception.h"
#include <iostream>
#include "echo_server.h"
#include "timeouts/deadline_container.h"


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
//    deadline_container dc;
//    auto it1 = dc.add(5, deadline_wrapper(5, time(0) + 5, 0));
////    dc.remove(it);
//    it1 = dc.update(it1, 10);
//    it1 = dc.update(it1, 12);
//
//    auto it2 = dc.add(5, deadline_wrapper(5, 15, 0));
//    it2 = dc.update(it2, 17);
//    it2 = dc.update(it2, 18);
//
//    auto it3 = dc.add(10, deadline_wrapper(10, 15, 0));
//    it3 = dc.update(it3, 16);
}
