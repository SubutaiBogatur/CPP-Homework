//
// Created by Aleksandr Tukallo on 09.04.17.
//

#include "timeouts/deadline_container.h"
#include "timeouts/deadline_container_tester.h"

int main()
{
    tester::do_operations(deadline_container(), 1000000, 100);
}
