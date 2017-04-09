//
// Created by Aleksandr Tukallo on 09.04.17.
//

#ifndef TIMEOUT_CONTAINER_H
#define TIMEOUT_CONTAINER_H

#define _GLIBCXX_USE_CXX11_ABI 0 //for nice gdb debugging

#include <list>
#include <map>
#include "deadline_wrapper.h"

/**
 * Structure is needed to store upcoming deadlines.
 */
struct deadline_container
{
public:
    typedef std::list<deadline_wrapper>::iterator list_iterator;
    typedef std::multimap<time_t, std::shared_ptr<std::list<deadline_wrapper>>>::iterator multimap_iterator;
    typedef std::shared_ptr<std::list<deadline_wrapper>> list_ptr;

private:
    //map stores pairs (timeout, queue)
    std::map<int, list_ptr> timeouts;

    //map stores pairs (timeout, iterator to pair with this queue in multimap)
    std::map<int, multimap_iterator> multimap_iterators;

    //multimap stores pairs (deadline in head of queue, queue)
    //  here map is needed to get minimum deadline in O(1) and to
    //  do insertions and deletions in logarithmic time
    std::multimap<time_t, list_ptr> deadlines;

    //method erases it from list and inserts it with new deadline in the end
    //  iterator to inserted node is returned
    list_iterator update_list(list_ptr list, list_iterator it, time_t new_deadline);

public:
    //method adds new deadline_wrapper to queue with given timeout
    //  post: node will be inserted in the end of respective queue
    list_iterator add(int timeout, deadline_wrapper node);

    //method removes client from queue. If queue with such timeout becomes
    //  empty, it is also removed
    void remove(list_iterator it);

    //given client is updated: it's deadline is changed to new one,
    //  though timeout remains the same. Method moves it to back of
    //  queue and updates structure of multimap (does erase, insert)
    //  to support multimap sorted
    //  pre: new_deadline is bigger, than an old one
    list_iterator update(list_iterator it, time_t new_deadline);

    //method returns pointer to closest deadline
    list_iterator get_min();
};

#endif // TIMEOUT_CONTAINER_H
