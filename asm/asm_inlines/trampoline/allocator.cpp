#include "allocator.h"
#include <sys/mman.h>

void *allocator::malloc()
{
    //move head forward
    void* ret = allocator::head;
    head = *(void**)head;
    return ret;
}

void allocator::free(void *ptr)
{
    //move head backwards
    *(void**) ptr = head;
    head = (void**) ptr;
}

//mmaps the page and makes it a list of free nodes
allocator::allocator()
{
    allocated = mmap(0, PAGE_SIZE * NUM_PAGES,
                     PROT_EXEC | PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    head = (void**) allocated;
    //link list
    for (size_t i = 0; i < PAGE_SIZE * NUM_PAGES; i += TRAMPOLINE_SIZE)
    {
        auto prev_node = (char*)allocated + i;
        *(void**)prev_node = 0;
         if (i != 0)
         {
             *(void**)(prev_node - TRAMPOLINE_SIZE) = prev_node;
         }
    }
}

allocator::~allocator()
{
    munmap(allocated, PAGE_SIZE * NUM_PAGES);
}

allocator &allocator::get_instance()
{
    static allocator static_instance = allocator();
    return static_instance;
}

