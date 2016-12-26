# CPP-Homework
The repository contains homeworks for C++ course in IFMO university.

* ASM contains some basic programs written in assembler: *addition*, *subtraction* and *multiplication* for long arithmetics. 

* `big_integer` is a class for long arithmetics in C++. It is based on a vector of unsigned integers and is implemented using optimizations: *small number optimization* and *copy-on-write optimization*. Class allows to do a lot of operations with big numbers effectively.

* `lru_cache` is a data structure for storing *Most Recently Used* elements. If the insertion in cache is made and the size becomes bigger, than capacity, the *Least Recently Used* element in cache is replaced by a new one. Data structure is implemented using a binary search tree, where all of the nodes are at the same time connected in a doubly linked list. 

* `checked` class is a wrapper aroung signed and unsigned integer types, which throws the exception, if overflow happens. This class was written as a homework to learn *Tag Dispatching* and *Traits classes*. 

* `my_bind` function is an implementation of [std::bind](http://en.cppreference.com/w/cpp/utility/functional/bind) function. It uses *move semantics* and *universal references* a lot. Should be compiled with `-std=c++14` flag because some functions return `auto` for better readability.
