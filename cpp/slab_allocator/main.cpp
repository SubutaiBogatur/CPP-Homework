#include <iostream>
#include "slab_allocator.hpp"
#include <vector>
#include <list>

struct foo {
    int a;
    int b;
    foo(int a, int b) : a(a), b(b) {}
};

//int main() {
//    std::cout << "finished" << std::endl;
//}