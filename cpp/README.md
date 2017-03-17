# Projects and homeworks for C++ course in ITMO University

* `big_integer` is a class for long arithmetics in C++. It is based on a vector of unsigned integers and is implemented using optimizations: *small number optimization* and *copy-on-write optimization*. Class allows to do a lot of operations with big numbers effectively. It is a first semester work.

* `lru_cache` is a data structure for storing *Most Recently Used* elements. If the insertion in cache is made and the size becomes bigger, than capacity, the *Least Recently Used* element in cache is replaced by a new one. Data structure is implemented using a binary search tree, where all of the nodes are at the same time connected in a doubly linked list. This was a task to write to pass an exam. 

* `checked` class is a wrapper aroung signed and unsigned integer types, which throws an exception, if overflow happens. This class was written as a homework to learn *Tag Dispatching* and *Traits classes*. 

* `my_bind` function is an implementation of [std::bind](http://en.cppreference.com/w/cpp/utility/functional/bind) function. It uses *move semantics* and *universal references* a lot. Should be compiled with `-std=c++14` flag because some functions return `auto` for better readability.

* `either` is a small version of `std::variant` standart container. It allows to store objects of two different types in one place in memory (while variant offers same functionality but with unlimited number of types). It is in many ways simular to `union` for two different types. Some functions in class are implemented using *double buffering* technique to achieve strong exception safety guarantee. Class was written as an exam task for second semester. 
