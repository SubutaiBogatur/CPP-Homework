//
// Created by Aleksandr Tukallo on 09.04.17.
//

#ifndef PROXY_SERVER_DEADLINE_CONTAINER_TESTER_H
#define PROXY_SERVER_DEADLINE_CONTAINER_TESTER_H

#include "deadline_container.h"

namespace tester
{
    //client is considered and int
    void do_operations(deadline_container dc = deadline_container(), int num_of_operations = 10,
                       int distribution_of_timeouts = 2);
}


#endif //PROXY_SERVER_DEADLINE_CONTAINER_TESTER_H
