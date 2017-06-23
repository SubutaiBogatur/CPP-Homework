#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <cstdio>

//Class implements extremely simple free-list allocator. All nodes have the same size
//  and constant number of pages is used. Allocator is a singleton and is just like
//  global variable, consider yourself warned
class allocator
{
public:
    void *malloc();
    void free(void *ptr);
    static allocator& get_instance();

private:
    static const size_t NUM_PAGES = 1;
    static const size_t PAGE_SIZE = 4096;
    static const size_t TRAMPOLINE_SIZE = 256;

    void *allocated; // points to the beginning of the mapped area
    void *head; // points to first free node in a list

    allocator();
    ~allocator();
};

#endif // ALLOCATOR_H
