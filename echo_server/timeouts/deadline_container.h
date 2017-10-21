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
 * @brief Structure is needed to store upcoming deadlines.
 *
 * Structure is implemented using \c std::map and \c std::multimap. They provide access to queues
 * with different timeouts in logarithmic time, which is totally acceptable considering possible
 * number of different timeouts.
 *
 * Structure should be used in pair with \c epoll_wait to easily get and update closest deadline.
 *
 * @author Aleksandr Tukallo
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

    /** @brief Method adds new deadline to queue with given timeout, \c O(logn)
     *
     * Needed queue is found in map of queues and given node is inserted in
     * the tail of it.
     *
     * @pre Deadline in node is greater, than every other deadline in
     * respective queue
     *
     * @param timeout determines the queue, where the node will be inserted
     * @param node is the node to insert
     * @return list iterator pointing to inserted node is returned
     */
    list_iterator add(int timeout, deadline_wrapper node);

    /** @brief Method removes node from respective queue, \c O(logn).
     *
     * @post \c is invalidated
     *
     * @param it iterator to node to delete
     */
    void remove(list_iterator it);

    /** @brief \c it is removed from queue and then reinserted with new deadline, \c O(logn)
     *
     * Given wrapper is changed: it's timeout remains the same, so it remains in the same
     * queue, but it's deadline is updated in such a way, that wrapper is moved to the tail
     * of the queue.
     *
     * @pre \c new_deadline is greater, than every other deadline in respective queue
     *
     * @post \c it is invalidated
     *
     * @param it specifies wrapper to update
     * @param new_deadline is deadline, set to the wrapper, after it is moved to the tail
     * @return iterator to updated wrapper is returned, it doesn't equal \c it
     */
    list_iterator update(list_iterator it, time_t new_deadline);

    /** @brief method returns smallest deadline, \c O(1)
     *
     * Throws \c server_exception if container is empty
     *
     * @return iterator to smallest element is returned
     */
    list_iterator get_min();

    /**
     * @brief returns true if there are no wrappers at all currently in container, \c O(1)
     */
    bool is_empty();

    ~deadline_container();
};

#endif // TIMEOUT_CONTAINER_H
