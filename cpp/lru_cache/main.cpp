#include <iostream>
#include "lru-cache.h"

void insert_erase_test(size_t number_of_tests);

int main()
{
    lru_cache tmp(5);

    tmp.insert({3, "a"});
    tmp.insert({4, "a"});
    tmp.insert({5, "a"});
    tmp.insert({1, "a"});
    tmp.insert({8, "a"});
    tmp.insert({7, "a"});

    lru_cache::iterator it = tmp.begin();

    if (it == tmp.end())
        std::cout << "ww";
    if (it != tmp.end())
        std::cout << "yy";

    value_type val;
    val = *it;
    it++;
    val = *it;
    it--;
    val = *it;

    it = (tmp.insert({0, "a"})).first;
    tmp.insert({6, "a"});

    it = tmp.find(1);
    it = tmp.find(2);
    it = tmp.find(3);
    it = tmp.find(6);
    it = tmp.find(8);

    insert_erase_test(100000);

    return 0;
}

#include <ctime>

void insert_erase_test(size_t number_of_tests)
{
    int const max_key = number_of_tests * 10;
    std::srand(std::time(0));

    lru_cache subject(number_of_tests);
    for (size_t i = 0; i < subject.get_capacity() * 2; i++)
    {
        uint32_t random_value = rand() % max_key;
        std::pair<lru_cache::iterator, bool> p;
        std::cout << "inser: " << random_value;
        p = subject.insert({random_value, "aaa"});
        std::cout << " bool: " << p.second << " size: " << subject.get_size() << std::endl;
    }

    for (size_t i = 0; i < subject.get_capacity() * 2; i++)
    {
        uint32_t random_value = rand() % max_key;
        if (subject.find(random_value) != subject.end())
        {
            std::cout << "erase " << random_value;
            subject.erase(subject.find(random_value));
            std::cout << " size: " << subject.get_size() << std::endl;
        }
    }

}



