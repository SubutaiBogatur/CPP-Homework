//
// Created by Aleksandr Tukallo on 12.06.16.
//


#include"big_integer.h"
#include<algorithm>
#include <iostream>

//invariant:
// arr.size() equals number of digits of big_int in BASE

big_integer::big_integer()
{
    this->sign = 0;
    this->arr.resize(1);
    this->arr[0] = 0;
}

//copy constructor
big_integer::big_integer(big_integer const& other)
{
    this->arr = other.arr;
    this->sign = other.sign;
}

big_integer::big_integer(int32_t other)
{
    int64_t tmp = other; //not to get overflow when other is Min_Value of int
    this->sign = 0;
    if (tmp < 0)
    {
        this->sign = 1;
        tmp = -tmp;
    }
    this->arr.resize(1);
    this->arr[0] = static_cast<uint32_t>(tmp);
}

big_integer::big_integer(std::string const& str)
{
    size_t j = 0;
    if (str[0] == '-')
    {
        j++;
        this->sign = 1;
    } else
        this->sign = 0;

    this->arr.resize(1);
    this->arr[0] = 0;

    for (size_t i = j; i < str.size(); i++)
    {
        *this = this->mul_long_short_unsigned(10);
        *this = this->add_long_short_unsigned(static_cast<uint32_t>(str[i] - '0'));
    }
}

std::string to_string(big_integer const& a)
{
    big_integer tmp = a;
    std::string out = "";
    if (tmp.is_zero())
    {
        out.push_back('0');
        return out;
    }
    while (!tmp.is_zero())
        out.push_back(static_cast<char>(tmp.get_remainder(10) + '0'));

    if (tmp.sign == 1)
        out.push_back('-');

    std::reverse(out.begin(), out.end());
    return out;
}

big_integer& big_integer::operator=(big_integer const& other)
{
    this->arr = other.arr;
    this->sign = other.sign;
    return (*this);
}

bool operator==(big_integer const& a, big_integer const& b)
{
    return (a.compare(b) == big_integer::compare_result::equals);
}

bool operator!=(big_integer const& a, big_integer const& b)
{
    return (a.compare(b) != big_integer::compare_result::equals);
}

bool operator<(big_integer const& a, big_integer const& b)
{
    return (a.compare(b) == big_integer::compare_result::less);
}

bool operator>(big_integer const& a, big_integer const& b)
{
    return (a.compare(b) == big_integer::compare_result::greater);
}

bool operator<=(big_integer const& a, big_integer const& b)
{
    return (a.compare(b) != big_integer::compare_result::greater);
}

bool operator>=(big_integer const& a, big_integer const& b)
{
    return (a.compare(b) != big_integer::compare_result::less);
}

big_integer& big_integer::operator+=(big_integer const& rhs)
{
    return recognize_operation('+', rhs);
}

big_integer operator+(big_integer a, big_integer const& b)
{
    return a += b;
}

big_integer& big_integer::operator-=(big_integer const& rhs)
{
    return recognize_operation('-', rhs);
}

big_integer operator-(big_integer a, big_integer const& b)
{
    return a -= b;
}

big_integer& big_integer::operator*=(big_integer const& rhs)
{
    return recognize_operation('*', rhs);
}

big_integer operator*(big_integer a, big_integer const& b)
{
    return a *= b;
}

big_integer& big_integer::operator/=(big_integer const& rhs)
{
    return recognize_operation('/', rhs);
}

big_integer operator/(big_integer a, big_integer const& b)
{
    return a /= b;
}

big_integer& big_integer::operator%=(big_integer const& rhs)
{
    return *this -= ((*this / rhs) * rhs);
}

big_integer operator%(big_integer a, big_integer const& b)
{
    return a %= b;
}

big_integer big_integer::operator+() const
{
    return *this;
}

big_integer big_integer::operator-() const
{
    big_integer tmp(*this);
    tmp.sign = !tmp.sign;
    return tmp;
}

big_integer big_integer::operator~() const  //nor
{
    big_integer tmp(*this);
    tmp += 1;
    tmp.sign = !tmp.sign;
    return tmp;
}

big_integer& big_integer::operator++() //prefix
{
    big_integer tmp(1);
    *this += tmp;
    return *this;
}

big_integer big_integer::operator++(int) //postfix
{
    big_integer tmp = *this;
    ++*this;
    return tmp;
}

big_integer& big_integer::operator--()
{
    big_integer tmp(1);
    *this -= tmp;
    return *this;
}

big_integer big_integer::operator--(int)
{
    big_integer tmp = *this;
    --*this;
    return tmp;
}

//binary operators
static uint32_t and_func(uint32_t const& a, uint32_t const& b)
{
    return a & b;
}

static uint32_t or_func(uint32_t const& a, uint32_t const& b)
{
    return a | b;
}

static uint32_t xor_func(uint32_t const& a, uint32_t const& b)
{
    return a ^ b;
}

big_integer& big_integer::operator&=(big_integer const& rhs)
{
    return this->binary_operation(and_func, rhs);
}

big_integer operator&(big_integer a, big_integer const& b)
{
    return a &= b;
}

big_integer& big_integer::operator|=(big_integer const& rhs)
{
    return this->binary_operation(or_func, rhs);
}

big_integer operator|(big_integer a, big_integer const& b)
{
    return a |= b;
}

big_integer& big_integer::operator^=(big_integer const& rhs)
{
    return this->binary_operation(xor_func, rhs);
}

big_integer operator^(big_integer a, big_integer const& b)
{
    return a ^= b;
}

big_integer& big_integer::operator<<=(int rhs) //shl
{
    if (rhs == 0 || is_zero()) return *this;
    if (rhs < 0) return this->operator>>=(-rhs);

    uint32_t parts = static_cast<uint32_t>(rhs) / 32;
    uint32_t bits = static_cast<uint32_t>(rhs) % 32;
    this->arr.push_back(0);

    if (bits != 0)
    {
        uint32_t mask = ~((1U << (32U - bits)) - 1U);
        for (size_t i = this->arr.size() - 1; i > 0; i--)
        {
            mask &= this->arr[i - 1];
            mask >>= (32U - bits);

            uint32_t cur_part;
            cur_part = this->arr[i - 1];
            cur_part <<= bits;

            this->arr[i - 1] = cur_part;
            this->arr[i] |= mask;
        }
    }
    this->arr.resize(this->arr.size() + parts);

    for (size_t i = this->arr.size(); i > static_cast<size_t>(parts); i--)
        this->arr[i - 1] = this->arr[i - parts - 1]; //-1, because unsigned

    for (size_t i = 0; i < parts; i++)
        this->arr[i] = 0;

    this->erase_leading_zeroes();
    return *this;
}

big_integer operator<<(big_integer a, int b)
{
    return a <<= b;
}

big_integer& big_integer::operator>>=(int rhs) //shr
{
    if (rhs == 0 || is_zero()) return *this;
    if (rhs < 0) return this->operator<<=(-rhs);

    uint32_t parts = static_cast<uint32_t>(rhs) / 32;
    uint32_t bits = static_cast<uint32_t>(rhs) % 32;

    this->convert_to_additional_code();

    if (bits != 0)
    {
        uint32_t mask = ~((1U << (32U - bits)) - 1U);
        for (size_t i = parts; i < this->arr.size(); i++)
        {
            uint32_t cur_part = this->arr[i] >> bits;
            if ((i == this->arr.size() - 1) && this->sign)
                cur_part |= mask;

            this->arr[i] = cur_part;
            if (i < this->arr.size() - 1)
            {
                uint32_t prev = (1U << bits) - 1;
                prev &= arr[i + 1];
                prev <<= (32 - bits);
                arr[i] |= prev;
            }
        }
    }

    for (size_t i = parts; i < arr.size(); i++)
        this->arr[i - parts] = this->arr[i];

    uint32_t mask = (!this->sign ? 0 : std::numeric_limits<uint32_t>::max());
    for (size_t i = 0; i < parts; i++)
        this->arr[this->arr.size() - i - 1] = mask;

    this->convert_to_additional_code();
    this->erase_leading_zeroes();

    return *this;
}

big_integer operator>>(big_integer a, int b)
{
    return a >>= b;
}

//converts to additional code
inline big_integer& big_integer::convert_to_additional_code()
{
    if (this->sign) //if negative
    {
        (*this)++; //++ to negative is sub by abs
        for (size_t i = 0; i < arr.size(); i++)
            arr[i] = ~arr[i];

        //this->sign = 0;
    }
    return *this;
}

//pre:
// gets '&', '|' or '^' as operand
big_integer& big_integer::binary_operation(uint32_t (*ptr)(uint32_t const&, uint32_t const&), big_integer const& other)
{
    big_integer right(other);
    this->convert_to_additional_code();
    right.convert_to_additional_code();

    big_integer const *maxptr, *minptr;
    compare_length(*this, right, &maxptr, &minptr);
    this->arr.resize(maxptr->arr.size());

    for (size_t i = 0; i < minptr->arr.size(); i++)
        this->arr[i] = (*ptr)(maxptr->arr[i], minptr->arr[i]);

    for (size_t i = minptr->arr.size(); i < maxptr->arr.size(); i++)
        this->arr[i] = (*ptr)(maxptr->arr[i], (!minptr->sign ? 0 : std::numeric_limits<uint32_t>::max()));

    this->sign = static_cast<bool>((*ptr)(static_cast<uint32_t>(this->sign), static_cast<uint32_t>(right.sign)));

    this->convert_to_additional_code();
    this->erase_leading_zeroes();
    return *this;
}

//pre:
// gets *this b_i
//post:
// returns:
// 1, if zero, sign doesn't matter
// 0, if not zero
inline bool big_integer::is_zero() const
{
    return ((this->arr.size() == 1) && (this->arr[0] == 0));
}

big_integer::compare_result big_integer::compare(big_integer const& a) const
{
    if (this->is_zero() && a.is_zero()) return compare_result::equals;
    if (this->sign < a.sign) return compare_result::greater; //0 < 1, 0 is positive
    if (this->sign > a.sign) return compare_result::less;
    if (this->sign == 0) //positive
    {
        return this->compare_by_abs(a);
    }

    if (this->arr.size() > a.arr.size()) return compare_result::less;
    if (this->arr.size() < a.arr.size()) return compare_result::greater;
    for (size_t i = this->arr.size(); i > 0; i--)
    {
        if (this->arr[i - 1] != a.arr[i - 1])
        {
            if (this->arr[i - 1] > a.arr[i - 1]) return compare_result::less;
            return compare_result::greater;
        }
    }
    return compare_result::equals;
}

big_integer::compare_result big_integer::compare_by_abs(big_integer const& a) const
{
    if (this->arr.size() > a.arr.size()) return compare_result::greater;
    if (this->arr.size() < a.arr.size()) return compare_result::less;
    for (size_t i = this->arr.size(); i > 0; i--)
    {
        if (this->arr[i - 1] != a.arr[i - 1])
        {
            if (this->arr[i - 1] > a.arr[i - 1]) return compare_result::greater;
            return compare_result::less;
        }
    }
    return compare_result::equals;
}

//removes all leading zeroes, except the only one, if the value of b_i is zero. 
inline void big_integer::erase_leading_zeroes()
{
    for (size_t i = this->arr.size(); i > 1; i--)
    {
        if (this->arr[i - 1] == 0)
            this->arr.pop_back();
        else
            break;
    }
}

//pre:
// gets '+', '-', '*' or '/' as operand
big_integer& big_integer::recognize_operation(char operand, big_integer const& other)
{
    if (operand == '+')
    {
        if (this->sign == 0 && other.sign == 0)
            return this->add_long_long_unsigned(other);
        if (this->sign == 0 && other.sign == 1)
            return this->correct_subtraction(other);
        if (this->sign == 1 && other.sign == 0)
        {
            if (this->compare_by_abs(other) == big_integer::compare_result::greater)
            {
                this->sub_long_long_unsigned(other);
                this->sign = 1;
            } else
            {
                big_integer tmp(other);
                tmp.sub_long_long_unsigned(*this); //reverse sub
                tmp.sign = 0;
                *this = tmp;
            }
        } else if (this->sign == 1 && other.sign == 1)
        {
            this->add_long_long_unsigned(other);
            this->sign = 1;
        }
        return *this;
    }
    if (operand == '-')
    {
        if (this->sign == 0 && other.sign == 0)
            return this->correct_subtraction(other);
        if (this->sign == 0 && other.sign == 1)
            return this->add_long_long_unsigned(other);
        if (this->sign == 1 && other.sign == 0)
        {
            this->add_long_long_unsigned(other);
            this->sign = 1;
        } else if (this->sign == 1 && other.sign == 1)
        {
            if (this->compare_by_abs(other) == big_integer::compare_result::greater)
            {
                this->sub_long_long_unsigned(other);
                this->sign = 1;
            } else
            {
                big_integer tmp(other);
                tmp.sub_long_long_unsigned(*this);
                tmp.sign = 0;
                *this = tmp;
            }
        }
        return *this;
    }
    if (this->sign ^ other.sign) //((this->sign == 0 && other.sign == 1) || (this->sign == 1 && other.sign == 0))
        this->sign = 1;
    else
        this->sign = 0;

    if (operand == '*')
        this->mul_long_long_unsigned(other);
    if (operand == '/')
        this->div_long_long_unsigned(other);

    return *this;
}

inline big_integer& big_integer::correct_subtraction(big_integer const& other)
{
    if (this->compare_by_abs(other) == big_integer::compare_result::greater)
    {
        this->sub_long_long_unsigned(other);
        this->sign = 0;
        return *this;
    } else
    {
        big_integer tmp(other);
        tmp.sub_long_long_unsigned(*this);
        tmp.sign = 1;
        *this = tmp;
        return *this;
    }
}

//static friend function. It can not be called from other .cpp files, and it has access to private fields.
static void
compare_length(big_integer const& a, big_integer const& b, big_integer const **maxptr, big_integer const **minptr)
{
    if (a.arr.size() > b.arr.size())
    {
        (*maxptr) = &a;
        (*minptr) = &b;
    } else
    {
        (*maxptr) = &b;
        (*minptr) = &a;
    }
}

//returns other sum of this and other: 2 positive b_i. The result is being put in *this and reference to this is returned.
big_integer& big_integer::add_long_long_unsigned(big_integer const& other)
{
    if (other.arr.size() == 1)
        return this->add_long_short_unsigned(other.arr[0]);

    uint64_t cur_sum;
    uint64_t carry = 0;

    big_integer const *maxptr;
    big_integer const *minptr;
    compare_length(*this, other, &maxptr, &minptr);

    this->arr.resize(maxptr->arr.size());
    for (size_t i = 0; i < minptr->arr.size(); i++)
    {
        cur_sum = static_cast<uint64_t>(minptr->arr[i]) + static_cast<uint64_t>(maxptr->arr[i]) + carry;
        this->arr[i] = static_cast<uint32_t>(cur_sum % BASE);
        carry = cur_sum / BASE;
    }
    for (size_t i = minptr->arr.size(); i < maxptr->arr.size(); i++)
    {
        cur_sum = static_cast<uint64_t>(maxptr->arr[i]) + carry;
        this->arr[i] = static_cast<uint32_t>(cur_sum % BASE);
        carry = cur_sum / BASE;
    }
    if (carry != 0)
        this->arr.push_back(static_cast<uint32_t>(carry));

    this->erase_leading_zeroes();
    return *this;
}

big_integer& big_integer::add_long_short_unsigned(uint32_t const other)
{
    uint64_t cur_sum;
    uint64_t carry = other;
    size_t i = 0;
    while (carry)
    {
        cur_sum = this->arr[i] + carry;
        this->arr[i] = static_cast<uint32_t>(cur_sum % BASE);
        carry = cur_sum / BASE;
        i++;
        if (i == this->arr.size())
        {
            if (carry)
                this->arr.push_back(static_cast<uint32_t>(carry));
            break;
        }
    }
    return *this;
}

big_integer& big_integer::sub_long_long_unsigned(big_integer const& other)
{
    uint32_t carry = 0;
    int64_t cur_sub;
    for (size_t i = 0; i < other.arr.size() || carry; i++)
    {
        cur_sub = static_cast<uint64_t>(this->arr[i]) - static_cast<uint64_t>(carry) -
                  (i < other.arr.size() ? static_cast<uint64_t>(other.arr[i]) : 0);
        carry = static_cast<uint32_t>(cur_sub < 0);
        if (carry) cur_sub += static_cast<uint64_t>(BASE);
        this->arr[i] = static_cast<uint32_t>(cur_sub);
    }
    this->erase_leading_zeroes();
    return *this;
}

//multiply *this by b and put result in *this
big_integer& big_integer::mul_long_long_unsigned(big_integer const& b)
{
    if (b.arr.size() == 1)
        return this->mul_long_short_unsigned(b.arr[0]);

    big_integer const a(*this);

    this->arr.resize(a.arr.size() + b.arr.size() + 1);
    for (size_t p = 0; p < a.arr.size() + b.arr.size() + 1; p++)
        arr[p] = 0;

    uint64_t cur_mul, carry, cur_sum;
    for (size_t i = 0; i < a.arr.size(); i++)
    {
        carry = 0;
        for (size_t j = 0; j < b.arr.size(); j++)
        {
            cur_mul = static_cast<uint64_t>(a.arr[i]) * static_cast<uint64_t>(b.arr[j]);
            cur_sum = cur_mul + static_cast<uint64_t>(this->arr[i + j]) + carry;
            this->arr[i + j] = static_cast<uint32_t>(cur_sum % BASE);
            carry = cur_sum / BASE;
        }
        if (carry != 0)
            this->arr[i + b.arr.size()] = static_cast<uint32_t>(carry);
    }
    this->erase_leading_zeroes();
    return *this;
}

big_integer& big_integer::mul_long_short_unsigned(uint32_t const other)
{
    uint64_t cur_mul, carry = 0;
    for (size_t i = 0; i < this->arr.size(); i++)
    {
        cur_mul = static_cast<uint64_t>(this->arr[i]) * static_cast<uint64_t>(other) + carry;
        this->arr[i] = static_cast<uint32_t>(cur_mul % BASE);
        carry = cur_mul / BASE;
    }
    if (carry != 0)
        this->arr.push_back(static_cast<uint32_t>(carry));

    this->erase_leading_zeroes();
    return *this;
}

big_integer& big_integer::div_long_long_unsigned(big_integer const& b)
{
    big_integer::compare_result comparison = b.compare_by_abs(*this);
    if (comparison == big_integer::compare_result::greater) //b is bigger, than *this
    {
        this->arr.clear();
        this->arr.resize(1);
        this->arr[0] = 0;
        return *this;
    } else if (comparison == big_integer::compare_result::equals) //equal
    {
        this->arr.clear();
        this->arr.resize(1);
        this->arr[0] = 1;
        return *this;
    }

    if (b.arr.size() == 1)
    {
        if (this->arr.size() == 1) //div_short_short
        {
            uint32_t div_res = this->arr[0] / b.arr[0];
            this->arr.resize(1);
            this->arr[0] = div_res;
            return *this;
        } else //div_long_short. <- makes tests 10 times faster.
        {
            this->get_remainder(b.arr[0]);
            return *this;
        }
    }

    big_integer a(*this);
    this->arr.clear();
    this->arr.resize(a.arr.size() - b.arr.size() + 1);
    big_integer sub_res;

    big_integer tmp;
    tmp.arr.resize(b.arr.size());
    size_t x = a.arr.size() - b.arr.size();
    for (size_t j = 0; j < b.arr.size(); j++)//kind of range constructor
        tmp.arr[j] = a.arr[x + j];

    for (int32_t i = static_cast<int32_t>(x); i >= 0; i--) //i is signed, because it becomes negative finally
    {
        if (i != static_cast<int32_t>(x)) //if not first iteration
        {
            tmp = sub_res;
            tmp.arr.resize(tmp.arr.size() + 1);
            for (size_t k = tmp.arr.size() - 1; k > 0; k--) //move by one to right
                tmp.arr[k] = tmp.arr[k - 1];

            tmp.arr[0] = a.arr[i];
            tmp.erase_leading_zeroes();
        }

        //binsearch to find out such k : b * k = tmp .
        // k >= 0 && k < BASE
        int64_t l = -1;
        int64_t r = static_cast<int64_t>(BASE); //BASE is smaller, than MAX_INT64
        while (l < r - 1)
        {
            int64_t m = (l + r) / 2;
            big_integer b_copy(b);
            if ((b_copy.mul_long_short_unsigned(static_cast<uint32_t>(m))).compare_by_abs(tmp) != big_integer::compare_result::greater) //less or equal
                l = m;
            else
                r = m;
        }
        this->arr[i] = static_cast<uint32_t>(l);

        big_integer b_copy(b);
        sub_res = tmp.sub_long_long_unsigned(b_copy.mul_long_short_unsigned(static_cast<uint32_t>(l)));
    }

    this->erase_leading_zeroes();
    return *this;
}

//pre:
// gets int
//post:
// returns remainder of division of (*this) by other. return (*this_old) % other
// (*this) is being divided by other. (*this_new) = (*this_old) / other.
uint32_t big_integer::get_remainder(uint32_t const other)
{
    uint64_t cur_sum, carry = 0;
    for (size_t i = this->arr.size(); i > 0; i--)
    {
        cur_sum = static_cast<uint64_t>(this->arr[i - 1]) + carry * BASE;
        this->arr[i - 1] = static_cast<uint32_t>((cur_sum / static_cast<uint64_t>(other)));
        carry = cur_sum % static_cast<uint64_t>(other);
    }

    this->erase_leading_zeroes();
    return static_cast<uint32_t>(carry);
}

std::ostream& operator<<(std::ostream& s, big_integer const& a)
{
    return s << to_string(a);
}