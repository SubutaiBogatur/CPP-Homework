//
// Created by Aleksandr Tukallo on 09.04.17.
//

#include "deadline_container.h"
#include <limits>

//function is needed only for comfortable gdb debugging
static std::map<int, std::list<deadline_wrapper>>
get_debug_timeouts(std::map<int, std::shared_ptr<std::list<deadline_wrapper>>> m)
{
    std::map<int, std::list<deadline_wrapper>> ret;
    for (auto it = m.begin(); it != m.end(); it++)
    {
        ret.insert({it->first, *(it->second.get())});
    }
    return ret;
};

//only for gdb debugging
static std::multimap<time_t, std::list<deadline_wrapper>>
get_debug_deadlines(std::multimap<time_t, std::shared_ptr<std::list<deadline_wrapper>>> m)
{
    std::multimap<time_t, std::list<deadline_wrapper>> ret;
    for (auto it = m.begin(); it != m.end(); it++)
    {
        ret.insert({it->first, *(it->second.get())});
    }
    return ret;
};

deadline_container::list_iterator deadline_container::add(int timeout, deadline_wrapper node)
{
    auto it = timeouts.find(timeout); //it points to queue to insert node in
    if (it == timeouts.end())
    {
        list_ptr sh_ptr = std::make_shared<std::list<deadline_wrapper>>();
        it = timeouts.insert({timeout, sh_ptr}).first; //add new queue for given timeout
        //next line is the worst line ever
        multimap_iterators.insert({timeout, deadlines.insert({node.deadline, it->second})});
    }
    it->second->push_back(node);
    return --it->second->end();
}

deadline_container::list_iterator deadline_container::update_list
        (deadline_container::list_ptr list, deadline_container::list_iterator it, time_t new_deadline)
{
    deadline_wrapper new_wrapper = deadline_wrapper(it->timeout, new_deadline, it->client);
    list->erase(it);
    list->push_back(new_wrapper);
    return --list->end();
}

deadline_container::list_iterator deadline_container::update(deadline_container::list_iterator it, time_t new_deadline)
{
    multimap_iterator mm_it = multimap_iterators.find(it->timeout)->second;
    if (mm_it->second->begin() == it)
    {
        //if deadlines must be edited
        list_ptr queue = mm_it->second;
        list_iterator ret = update_list(queue, it, new_deadline);
        deadlines.erase(mm_it);
        multimap_iterators.erase(it->timeout);
        //next line is another worst line ever
        multimap_iterators.insert({it->timeout, deadlines.insert({queue->begin()->deadline, queue})});
        return ret;
    }
    return update_list(mm_it->second, it, new_deadline);
}

//todo what if min not unique
deadline_container::list_iterator deadline_container::get_min()
{
    return deadlines.begin()->second->begin();
}

void deadline_container::remove(deadline_container::list_iterator it)
{
    it = update(it, std::numeric_limits<time_t>::max());
    if (timeouts.find(it->timeout)->second->size() == 1)
    {
        timeouts.erase(it->timeout);
        deadlines.erase(multimap_iterators.find(it->timeout)->second);
        multimap_iterators.erase(it->timeout);
    } else
    {
        //else because of update it is the last in the queue and we can just
        //  delete it without changing multimap
        timeouts.find(it->timeout)->second->erase(it);
    }
}

bool deadline_container::is_empty()
{
    return timeouts.size() == 0;
}





