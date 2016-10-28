//
// Created by Aleksandr Tukallo on 28.10.16.
//

#include <assert.h>
#include <iostream>
#include "optimized_vector.h"

void optimized_vector::copy_optimized_vector(optimized_vector const& other)
{
    if (other.is_small)
        this->number = other.number;
    else
        this->data_ptr = other.data_ptr;
    this->is_small = other.is_small;
}

optimized_vector::optimized_vector(optimized_vector const& other)
{
    copy_optimized_vector(other);
}

optimized_vector& optimized_vector::operator=(const optimized_vector& other)
{
    copy_optimized_vector(other);
    return *this;
}

//main function for copy-on-write optimization.
//when the vector is changed, if (*this) is not the only one, who uses vector,
//another copy of vector for (*this) is created
void optimized_vector::copy_if_not_unique()
{
    if(this->data_ptr.unique())
        return; //no copy made
    //else

    std::shared_ptr<std::vector<uint32_t>> tmp = data_ptr;
    data_ptr = std::make_shared<std::vector<uint32_t>>(*tmp); //copy is made
}

//pre:
// new_size is bigger, than current size
void optimized_vector::resize(size_t new_size)
{
    if(is_small && new_size == 1)
        return; //number already exists

    //else: working with vector
    if (is_small)
    {
        is_small = false;
        uint32_t tmp = number;
        data_ptr = std::make_shared<std::vector<uint32_t>>(new_size);
        data_ptr->at(0) = tmp;
        return;
    }
    //else:

    copy_if_not_unique();
    data_ptr->resize(new_size);
    return;
}

void optimized_vector::clear()
{
    //it was decided not to make number short again, if it once was already long.

    if (is_small)
    {
        number = 0;
        return;
    }
    //else
    copy_if_not_unique();
    this->data_ptr->clear();
}

const uint32_t& optimized_vector::operator[](size_t position) const
{
    if (is_small)
        return number;
    //else
    return data_ptr->at(position);
}

uint32_t& optimized_vector::operator[](size_t position)
{
    if (is_small)
        return number;
    //else
    copy_if_not_unique();
    return data_ptr->at(position);
}

void optimized_vector::push_back(const uint32_t& value)
{
    if (is_small)
    {
        is_small = false;
        uint32_t tmp = number;
        data_ptr = std::make_shared<std::vector<uint32_t>>(2);
        data_ptr->at(0) = tmp;
        data_ptr->at(1) = value;
        return;
    }
    //else
    copy_if_not_unique();
    data_ptr->push_back(value);
}

void optimized_vector::pop_back()
{
    assert(!is_small); //value is not small

    copy_if_not_unique();
    data_ptr->pop_back();
    return;
}

size_t optimized_vector::size() const
{
    if (is_small)
        return 1;
    else return data_ptr->size();
}