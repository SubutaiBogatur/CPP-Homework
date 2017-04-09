//
// Created by Aleksandr Tukallo on 09.04.17.
//

#include "deadline_container.h"

int main()
{
    deadline_container dc;
    auto it11 = dc.add(11, deadline_wrapper(11, 10, 1));
    auto it12 = dc.add(11, deadline_wrapper(11, 20, 1));
    auto it13 = dc.add(11, deadline_wrapper(11, 30, 1));

    auto it21 = dc.add(6, deadline_wrapper(6, 5, 1));
    auto it22 = dc.add(6, deadline_wrapper(6, 15, 1));
    auto it23 = dc.add(6, deadline_wrapper(6, 25, 1));
    auto it24 = dc.add(6, deadline_wrapper(6, 35, 1));

    dc.update(it22, 45);
    dc.update(it21, 55);
    dc.update(it11, 60);
    dc.update(it12, 70);
    dc.update(it23, 75);


}
