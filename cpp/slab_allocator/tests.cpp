//
// Created by atukallo on 3/21/19.
//

#include <cstddef>
#include <numeric>
#include <limits>
#include <functional>
#include <list>
#include <map>
#include <set>
#include <vector>
#include <iostream>
#include <algorithm>
#include <cassert>

#include "slab_allocator.hpp"

template<class T>
using test_allocator = allocators::slab<T, 4096>;

template<class T>
using sa_list = std::list<T, test_allocator<T>>;

template<class T, class U>
using sa_map = std::map<T, U, std::less<T>, test_allocator<std::pair<T, U>>>;

template<class T>
using sa_set = std::set<T, std::less<T>, test_allocator<T>>;


struct checker {
    checker() {
        ++counter;
    }

    ~checker() {
        --counter;
    }

    checker(checker const &) {
        ++counter;
    }

    checker &operator=(checker const &) {
        return *this;
    }

    static size_t counter;
};

size_t checker::counter = 0;

template<class list_t>
static void remove_even(list_t &l) {
    bool even = false;
    for (auto it = l.begin(); it != l.end();) {
        if (even)
            it = l.erase(it);
        else
            ++it;

        even = !even;
    }
}

static void test_one_int() {
    sa_list<int> l;
    l.push_back(5);
    l.push_back(6);
}

static void test_string() {
    sa_list<std::string> l;
    for (size_t i = 0; i < 12644; ++i)
        l.push_back(std::to_string(i) + "sdfjknasljkfhasdkfhasdklfhklasdhjflasdfhas");

    remove_even(l);
}

static void test_checker() {
    sa_list<checker> l2;
    for (size_t i = 0; i < 12644; ++i)
        l2.push_back(checker());

    remove_even(l2);
}

static void test_alignment() {
    test_allocator<char> slab;

    for (size_t lg2i = 0; lg2i <= 15; ++lg2i) {
        const size_t page_size = 4 * 1024;
        const size_t size = 1ul << lg2i;
        const uintptr_t mask = std::max<uintptr_t>(8, std::min(page_size, size)) - 1;

        for (size_t j = 0; j < 1000; ++j) {
            auto p = slab.allocate(size);

            uintptr_t ptr = reinterpret_cast<uintptr_t>(p);
            assert((ptr & mask) == 0);
            slab.deallocate(p, size);
        }
    }
}

static void test_zero_allocation() {
    test_allocator<char> slab;

    auto p1 = slab.allocate(0);
    auto p2 = slab.allocate(0);
    assert(p1 != nullptr && p2 != nullptr);
    assert(p1 != p2);

    slab.deallocate(p1, 0);
    slab.deallocate(p2, 0);
}

static void test_deallocation() {
    test_allocator<char> slab;
    std::vector<std::allocator_traits<test_allocator<char>>::pointer> ps(10000);

    for (int i = 0; i < 100; ++i) {
        for (auto &p: ps)
            p = slab.allocate(100);
        for (auto &p: ps)
            slab.deallocate(p, 100);
    }
}

//int main() {
//    test_one_int();
//    test_string();
//    test_checker();
//    test_alignment();
//    test_zero_allocation();
//    test_deallocation();
//
//    std::cout << "Tests passed!" << std::endl;
//}
