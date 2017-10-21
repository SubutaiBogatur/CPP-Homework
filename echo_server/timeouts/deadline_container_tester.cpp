//
// Created by Aleksandr Tukallo on 09.04.17.
//

#include "deadline_container_tester.h"
#include <set>
#include <assert.h>
#include <iostream>
#include <limits>

static time_t get_highest_deadline(int timeout, std::multimap<int, std::list<deadline_wrapper>::iterator>& deadlines)
{
    time_t max = 0;
    for (auto it = deadlines.begin(); it != deadlines.end(); it++)
    {
        if (it->second->timeout == timeout && it->second->deadline > max)
        {
            max = it->first;
        }
    }
    return max;
}

static time_t get_min(std::multimap<int, std::list<deadline_wrapper>::iterator>& deadlines)
{
    time_t min = std::numeric_limits<time_t>::max();
    for (auto it = deadlines.begin(); it != deadlines.end(); it++)
    {
        if (it->first < min)
        {
            min = it->first;
        }
    }
    return min;
}

void ::tester::do_operations(deadline_container dc, int num_of_operations, int distribution_of_timeouts)
{
    std::srand(time(NULL));

    std::multimap<int, std::list<deadline_wrapper>::iterator> deadlines;

    for (int i = 0; i < num_of_operations; i++)
    {
        //choose operation
        switch (deadlines.size() <= 10 ? 0 : std::rand() % 3)
        {
            case 0:
            {
                //do addition
                int timeout = rand() % distribution_of_timeouts + 1;
                if (timeout == 70)
                {
                    std::cout << "70\n";
                }
                time_t deadline = get_highest_deadline(timeout, deadlines) + timeout;
                deadline_wrapper dw(timeout, deadline, 0);
                deadlines.insert({deadline, dc.add(timeout, dw)});
            }
                break;

            case 1:
            {
                //do updating
                auto it = deadlines.begin();
                advance(it, std::rand() % deadlines.size());
                time_t new_deadline = get_highest_deadline(it->second->timeout, deadlines) + it->second->timeout;
                deadlines.insert({new_deadline, dc.update(it->second, new_deadline)});
                deadlines.erase(it);
            }
                break;
            case 2:
            {
                //do removing
                auto it = deadlines.begin();
                advance(it, std::rand() % deadlines.size());
                dc.remove(it->second);
                deadlines.erase(it);
            }
                break;
        }
        if (deadlines.size() != 0)
        {
            assert (get_min(deadlines) == dc.get_min()->deadline);
        }
        std::cout << "Cur min: " << (deadlines.size() != 0 ? dc.get_min()->deadline : 0) << std::endl;
        if (get_min(deadlines) == 29)
        {
            std::cout << "here\n";
        }
    }
}
