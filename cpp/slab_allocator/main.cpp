#include <iostream>
#include "slab_allocator.hpp"
#include <vector>
#include <list>

int main() {
    std::list<int, slab_allocator<int>> l;

    for (size_t i = 0; i < 127; i++) {
        l.push_back(1);
    }

}