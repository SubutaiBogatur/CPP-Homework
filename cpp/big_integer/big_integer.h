//
// Created by Aleksandr Tukallo on 12.06.16.
//

#ifndef BIG_INTEGER_H
#define BIG_INTEGER_H

#include "optimized_vector.h"

struct big_integer;

//it must be declared static before friend.
static void
compare_length(big_integer const& a, big_integer const& b, big_integer const **maxptr, big_integer const **minptr);

struct big_integer
{
    big_integer();
    big_integer(big_integer const& other);
    big_integer(int32_t a);
    explicit big_integer(std::string const& str);

    big_integer& operator=(big_integer const& other);

    big_integer& operator+=(big_integer const& rhs);
    big_integer& operator-=(big_integer const& rhs);
    big_integer& operator*=(big_integer const& rhs);
    big_integer& operator/=(big_integer const& rhs);
    big_integer& operator%=(big_integer const& rhs);

    big_integer& operator&=(big_integer const& rhs);
    big_integer& operator|=(big_integer const& rhs);
    big_integer& operator^=(big_integer const& rhs);

    big_integer& operator<<=(int rhs);
    big_integer& operator>>=(int rhs);

    big_integer operator+() const;
    big_integer operator-() const;
    big_integer operator~() const;

    big_integer& operator++();
    big_integer operator++(int);

    big_integer& operator--();
    big_integer operator--(int);

    friend bool operator==(big_integer const& a, big_integer const& b);
    friend bool operator!=(big_integer const& a, big_integer const& b);
    friend bool operator<(big_integer const& a, big_integer const& b);
    friend bool operator>(big_integer const& a, big_integer const& b);
    friend bool operator<=(big_integer const& a, big_integer const& b);
    friend bool operator>=(big_integer const& a, big_integer const& b);

    friend std::string to_string(big_integer const& a);

private:

    enum compare_result {equals, greater, less};

    //member variables:
    optimized_vector arr;

    //static for BASE speeds up tests to almost TEN TIMES
    static const uint64_t BASE = static_cast<uint64_t>(UINT32_MAX) + 1;
    bool sign; //the same as bit for sign: zero = plus;

    //functions:
    inline bool is_zero() const;
    compare_result compare(big_integer const& a) const;
    compare_result compare_by_abs(big_integer const& a) const;
    big_integer& recognize_operation(char operand, big_integer const& other);
    inline big_integer& correct_subtraction(big_integer const& other);
    inline void erase_leading_zeroes();

    //Can not be defined as friend and static in one string. Static must be declared earlier.
    friend void
    compare_length(big_integer const& a, big_integer const& b, big_integer const **maxptr, big_integer const **minptr);

    big_integer& binary_operation(uint32_t (*)(uint32_t const&, uint32_t const&), big_integer const& other);
    inline big_integer& convert_to_additional_code();

    big_integer& add_long_long_unsigned(big_integer const& other);
    big_integer& add_long_short_unsigned(uint32_t const other);
    big_integer& sub_long_long_unsigned(big_integer const& other);
    big_integer& mul_long_long_unsigned(big_integer const& other);
    big_integer& mul_long_short_unsigned(uint32_t const other);
    big_integer& div_long_long_unsigned(big_integer const& other);
    uint32_t get_remainder(uint32_t const other); //kind of div_long_short
};

big_integer operator+(big_integer a, big_integer const& b);
big_integer operator-(big_integer a, big_integer const& b);
big_integer operator*(big_integer a, big_integer const& b);
big_integer operator/(big_integer a, big_integer const& b);
big_integer operator%(big_integer a, big_integer const& b);

big_integer operator&(big_integer a, big_integer const& b);
big_integer operator|(big_integer a, big_integer const& b);
big_integer operator^(big_integer a, big_integer const& b);

big_integer operator<<(big_integer a, int b);
big_integer operator>>(big_integer a, int b);

bool operator==(big_integer const& a, big_integer const& b);
bool operator!=(big_integer const& a, big_integer const& b);
bool operator<(big_integer const& a, big_integer const& b);
bool operator>(big_integer const& a, big_integer const& b);
bool operator<=(big_integer const& a, big_integer const& b);
bool operator>=(big_integer const& a, big_integer const& b);

std::string to_string(big_integer const& a);
std::ostream& operator<<(std::ostream& s, big_integer const& a);

#endif