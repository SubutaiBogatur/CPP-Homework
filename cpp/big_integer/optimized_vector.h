//
// Created by Aleksandr Tukallo on 28.10.16.
//

#ifndef OPTIMIZED_VECTOR_H
#define OPTIMIZED_VECTOR_H

#include <vector>
#include <iosfwd>
#include <cstdint>
#include <memory>

struct optimized_vector
{
public:
    optimized_vector() : is_small(true), number(0)
    {};

    ~optimized_vector()
    {}

    optimized_vector(optimized_vector const& other);
    optimized_vector& operator=(const optimized_vector& other);

    void resize(size_t new_size);

    const uint32_t& operator[](size_t position) const; //fully const, read only
    uint32_t& operator[](size_t position); //read & write

    void push_back(const uint32_t&);
    void pop_back();

    void clear();

    size_t size() const;

private:
    bool is_small;
    uint32_t number; //no union, because it is not recommended to use union for fields with nontrivial constructor
    std::shared_ptr<std::vector<uint32_t>> data_ptr;
    void copy_optimized_vector(optimized_vector const& other);
    void copy_if_not_unique();
};


#endif //OPTIMIZED_VECTOR_H
