#include"big_integer.h"
#include <string>
#include<algorithm>

//Code was written and tested by Aleksandr Tukallo with Microsoft Visual Studio 2013 as a homework for CPP-course in IFMO university.

//invariant:
// data.size() equals number of digits of big_int in BASE

big_integer::big_integer()
{
	this->sign = 0;
	this->data.resize(1);
	this->data[0] = 0;
}

//copy constructor
big_integer::big_integer(big_integer const& other) 
{
	this->data = other.data;
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
	this->data.resize(1);
	this->data[0] = tmp;
}

big_integer::big_integer(std::string const& str)
{
	size_t k = 0;
	if (str[0] == '-')
	{
		++k;
		this->sign = 1;
	}
	else
		this->sign = 0;

	this->data.push_back(0);
	for (size_t i = k; i < str.size(); i++)
	{
		*this = this->mul_long_short_unsigned(10);
		*this = this->add_long_short_unsigned(str[i] - '0');
	}
}

big_integer::~big_integer()
{
	//Nothing changes if we call ~vector. It was created without new, so it will delete itself automatically.
	//this->data.~vector();
	//We do not call vector destructor, 'cause big_integer calls destructors for all its fields, when being destroyed.
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
		out.push_back((char)(tmp.get_remainder(10) + '0'));

	if (tmp.sign == 1)
		out.push_back('-');

	std::reverse(out.begin(), out.end());
	return out;
}

big_integer& big_integer::operator=(big_integer const& other)
{
	this->data = other.data;
	this->sign = other.sign;
	return (*this);
}

bool operator==(big_integer const& a, big_integer const& b)
{
	return a.compare(b) == 0 ? true : false;
}

bool operator!=(big_integer const& a, big_integer const& b)
{
	return a.compare(b) != 0 ? true : false;
}

bool operator<(big_integer const& a, big_integer const& b)
{
	return a.compare(b) == -1 ? true : false;
}

bool operator>(big_integer const& a, big_integer const& b)
{
	return a.compare(b) == 1 ? true : false;
}

bool operator<=(big_integer const& a, big_integer const& b)
{
	return a.compare(b) != 1 ? true : false;
}

bool operator>=(big_integer const& a, big_integer const& b)
{
	return a.compare(b) != -1 ? true : false;
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

big_integer &big_integer::operator%=(big_integer const &rhs)
{
	return *this -= ((*this / rhs) * rhs);
}

big_integer operator%(big_integer a, big_integer const &b)
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

big_integer &big_integer::operator++() //Prefix
{
	big_integer tmp(1);
	*this += tmp;
	return *this;
}

big_integer big_integer::operator++(int) //Postfix
{
	big_integer tmp = *this;
	++*this;
	return tmp;
}

big_integer &big_integer::operator--()
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

big_integer &big_integer::operator&=(big_integer const &rhs)
{
	return this->binary_operation('&', rhs);
}

big_integer operator&(big_integer a, big_integer const &b)
{
	return a &= b;
}

big_integer &big_integer::operator|=(big_integer const &rhs)
{
	return this->binary_operation('|', rhs);
}

big_integer operator|(big_integer a, big_integer const &b)
{
	return a |= b;
}

big_integer &big_integer::operator^=(big_integer const &rhs)
{
	return this->binary_operation('^', rhs);
}

big_integer operator^(big_integer a, big_integer const &b)
{
	return a ^= b;
}

big_integer &big_integer::operator<<=(int rhs) //shl
{
	if (rhs == 0 || is_zero()) return *this;
	if (rhs < 0) return this->operator>>=(-rhs);
	uint32_t blocks = static_cast<uint32_t>(rhs) / 32;
	uint32_t bytes = static_cast<uint32_t>(rhs) % 32;
	this->data.push_back(0);

	if (bytes != 0)
	{
		for (size_t i = data.size() - 1; i > 0; i--)
		{
			uint32_t next, cur;
			next = ~((1U << (32U - bytes)) - 1U); //Mask
			next &= data[i - 1];
			next >>= (32 - bytes);

			cur = data[i - 1];
			cur <<= bytes;

			data[i - 1] = cur;
			data[i] |= next;
		}
	}

	size_t size0 = data.size();
	data.resize(data.size() + blocks);

	for (int i = size0 + blocks - 1; i >= static_cast<int>(blocks); --i) //Workaround for blocks = 0
		data[i] = data[i - blocks];
	
	for (size_t i = 0; i < blocks; ++i)
		data[i] = 0;

	erase_leading_zeroes();
	return *this;
}

big_integer operator<<(big_integer a, int b)
{
	return a <<= b;
}

big_integer &big_integer::operator>>=(int rhs) //shr
{
	if (rhs == 0 || is_zero()) return *this;
	if (rhs < 0) return this->operator<<=(-rhs);

	uint32_t blocks = static_cast<uint32_t>(rhs) / 32;
	uint32_t bytes = static_cast<uint32_t>(rhs) % 32;

	this->convert_to_additional_code();

	if (bytes != 0)
	{
		for (size_t i = blocks; i < data.size(); i++)
		{
			uint32_t cur, prev;
			cur = data[i];
			cur >>= bytes;
			if (i == data.size() - 1 && this->sign)
			{
				uint32_t mask = ~((1U << (32U - bytes)) - 1U); //Mask, where first BYTES bytes are 1
				cur |= mask;
			}
			data[i] = cur;
			if (i < data.size() - 1)
			{
				prev = (1U << bytes) - 1;
				prev &= data[i + 1];
				prev <<= (32 - bytes);
				data[i] |= prev;
			}
		}
	}

	for (size_t i = blocks; i < data.size(); i++)
		data[i - blocks] = data[i];

	uint32_t mask = (!this->sign ? 0 : std::numeric_limits<uint32_t>::max());
	for (size_t i = 0; i < blocks; i++)
		data[data.size() - 1 - i] = mask;

	this->convert_to_additional_code();
	erase_leading_zeroes();

	return *this;
}

big_integer operator>>(big_integer a, int b)
{
	return a >>= b;
}


//converts to additional code
big_integer& big_integer::convert_to_additional_code()
{
	if (this->sign) //if negative
	{
		(*this)++; //++ to negative is sub by abs
		for (size_t i = 0; i < data.size(); i++)
			data[i] = ~data[i];
		
		//this->sign = 0;
	}
	return *this;
}

big_integer& big_integer::binary_operation(char operand, big_integer const& other)
{
	big_integer right(other);
	this->convert_to_additional_code();
	right.convert_to_additional_code();

	big_integer const *maxptr, *minptr;
	compare_length(*this, right, &maxptr, &minptr);

	this->data.resize(maxptr->data.size());
	for (size_t i = 0; i < minptr->data.size(); i++)
	{
		switch (operand)
		{
		case '&':
			this->data[i] = maxptr->data[i] & minptr->data[i];
			break;
		case '|':
			this->data[i] = maxptr->data[i] | minptr->data[i];
			break;
		case '^':
			this->data[i] = maxptr->data[i] ^ minptr->data[i];
			break;
		default:
			break;
		}
	}

	for (size_t i = minptr->data.size(); i < maxptr->data.size(); i++)
	{
		switch (operand)
		{
		case '&':
			this->data[i] = maxptr->data[i] & (!minptr->sign ? 0 : std::numeric_limits<uint32_t>::max());
			break;
		case '|':
			this->data[i] = maxptr->data[i] | (!minptr->sign ? 0 : std::numeric_limits<uint32_t>::max());
			break;
		case '^':
			this->data[i] = maxptr->data[i] ^ (!minptr->sign ? 0 : std::numeric_limits<uint32_t>::max());
			break;
		default:
			break;
		}
	}
		
	switch (operand)
	{
	case '&':
		this->sign = (this->sign && right.sign);
		break;
	case '|':
		this->sign = (this->sign || right.sign);
		break;
	case '^':
		this->sign = (this->sign ^ right.sign);
		break;
	default:
		break;
	}

	this->convert_to_additional_code();
	this->erase_leading_zeroes();
	return *this;
}

//pre:
// gets *this b_i
//post:
// returns:
// 1, is zero, sign doesn't matter
// 0, if not zero
bool big_integer::is_zero() const
{
	return ((this->data.size() == 1) && (this->data[0] == 0));
}

//pre:
// gets *this b_i and a b_i
//post:
// returns:
// 0, if *this == a
// 1, if *this > a
// -1, if *this < a
int8_t big_integer::compare(big_integer const& a) const
{
	if (this->is_zero() && a.is_zero()) return 0;
	if (this->sign < a.sign) return 1; //0 < 1, 0 is positive
	if (this->sign > a.sign) return -1;
	if (this->sign == 0) //positive
	{
		return this->compare_by_abs(a);
	}

	if (this->data.size() > a.data.size()) return -1;
	if (this->data.size() < a.data.size()) return 1;
	for (size_t i = this->data.size(); i > 0; i--)
	{
		if (this->data[i - 1] != a.data[i - 1])
		{
			if (this->data[i - 1] > a.data[i - 1]) return -1;
			return 1;
		}
	}
	return 0;
}

// 0, if *this == a
// 1, if *this > a
// -1, if *this < a
int8_t big_integer::compare_by_abs(big_integer const& a) const
{
	if (this->data.size() > a.data.size()) return 1;
	if (this->data.size() < a.data.size()) return -1;
	for (size_t i = this->data.size(); i > 0; i--)
	{
		if (this->data[i - 1] != a.data[i - 1])
		{
			if (this->data[i - 1] > a.data[i - 1]) return 1;
			return -1;
		}
	}
	return 0;
}

//removes all leading zeroes, except the only one, if the value of b_i is zero. 
void big_integer::erase_leading_zeroes()
{
	for (size_t i = this->data.size(); i > 1; i--)
	{
		if (this->data[i - 1] == 0)
			this->data.pop_back();
		else
			break;
	}
}

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
			if (this->compare_by_abs(other) == 1)
			{
				this->sub_long_long_unsigned(other);
				this->sign = 1;
			}
			else
			{
				big_integer tmp(other);
				tmp.sub_long_long_unsigned(*this);
				tmp.sign = 0;
				*this = tmp;
			}
		}
		if (this->sign == 1 && other.sign == 1)
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
		}
		if (this->sign == 1 && other.sign == 1)
		{
			if (this->compare_by_abs(other) == 1)
			{
				this->sub_long_long_unsigned(other);
				this->sign = 1;
			}
			else
			{
				big_integer tmp(other);
				tmp.sub_long_long_unsigned(*this);
				tmp.sign = 0;
				*this = tmp;
			}
		}
		return *this;
	}
	if (this->sign == 0 && other.sign == 1 || this->sign == 1 && other.sign == 0)
		this->sign = 1;
	else
		this->sign = 0;

	if (operand == '*')
		this->mul_long_long_unsigned(other);
	if (operand == '/')
		this->div_long_long_unsigned(other);

	return *this;
}

big_integer& big_integer::correct_subtraction(big_integer const& other)
{
	if (this->compare_by_abs(other) == 1)
		return this->sub_long_long_unsigned(other);
	else
	{
		big_integer tmp(other);
		tmp.sub_long_long_unsigned(*this);
		tmp.sign = 1;
		*this = tmp;
		return *this;
	}
}

//static friend function. It can not be called from other .cpp files, and it has access to private fields.
static void compare_length(big_integer const& a, big_integer const& b, big_integer const **maxptr, big_integer const **minptr)
{
	if (a.data.size() > b.data.size())
	{
		(*maxptr) = &a;
		(*minptr) = &b;
	}
	else
	{
		(*maxptr) = &b;
		(*minptr) = &a;
	}
}

//returns other sum of this and other: 2 positive b_i. The result is being put in *this and reference to this is returned.
big_integer& big_integer::add_long_long_unsigned(big_integer const& other)
{
	if (other.data.size() == 1)
		return this->add_long_short_unsigned(other.data[0]);

	uint64_t cur_sum;
	uint64_t carry = 0;

	big_integer const *maxptr;
	big_integer const *minptr;
	compare_length(*this, other, &maxptr, &minptr);

	this->data.resize(maxptr->data.size());
	for (size_t i = 0; i < minptr->data.size(); i++)
	{
		cur_sum = (uint64_t)minptr->data[i] + (uint64_t)maxptr->data[i] + carry;
		this->data[i] = cur_sum % BASE;
		carry = cur_sum / BASE;
	}
	for (size_t i = minptr->data.size(); i < maxptr->data.size(); i++)
	{
		cur_sum = (uint64_t)maxptr->data[i] + carry;
		this->data[i] = cur_sum % BASE;
		carry = cur_sum / BASE;
	}
	if (carry != 0)
		this->data.push_back(carry);

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
		cur_sum = this->data[i] + carry;
		this->data[i] = cur_sum % BASE;
		carry = cur_sum / BASE;
		i++;
		if (i == this->data.size())
		{
			if (carry)
				this->data.push_back(carry);
			break;
		}
	}
	return *this;
}

big_integer& big_integer::sub_long_long_unsigned(big_integer const& other)
{
	uint32_t carry = 0;
	int64_t cur_sub;
	for (size_t i = 0; i < other.data.size() || carry; i++)
	{
		cur_sub = (int64_t)this->data[i] - (int64_t)carry - (i < other.data.size() ? (int64_t)other.data[i] : (int64_t)0);
		carry = cur_sub < 0;
		if (carry)  cur_sub += (int64_t)BASE;
		this->data[i] = cur_sub;
	}
	this->erase_leading_zeroes();
	return *this;
}

//multiply *this by b and put result in *this
big_integer& big_integer::mul_long_long_unsigned(big_integer const& b)
{
	if (b.data.size() == 1)
		return this->mul_long_short_unsigned(b.data[0]);

	big_integer const a(*this);
	this->data.clear();
	this->data.resize(a.data.size() + b.data.size() + 1, 0);

	uint64_t cur_mul, carry, cur_sum;
	for (size_t i = 0; i < a.data.size(); i++)
	{
		carry = 0;
		for (size_t j = 0; j < b.data.size(); j++)
		{
			cur_mul = (uint64_t)a.data[i] * (uint64_t)b.data[j];
			cur_sum = cur_mul + (uint64_t)this->data[i + j] + carry;
			this->data[i + j] = cur_sum % BASE;
			carry = cur_sum / BASE;
		}
		if (carry != 0)
			this->data[i + b.data.size()] = carry;
	}
	this->erase_leading_zeroes();
	return *this;
}

big_integer& big_integer::mul_long_short_unsigned(uint32_t const other)
{
	uint64_t cur_mul, carry = 0;
	for (size_t i = 0; i < this->data.size(); i++)
	{
		cur_mul = (uint64_t)this->data[i] * (uint64_t)other + carry;
		this->data[i] = cur_mul % BASE;
		carry = cur_mul / BASE;
	}
	if (carry != 0)
		this->data.push_back(carry);

	this->erase_leading_zeroes();
	return *this;
}

big_integer& big_integer::div_long_long_unsigned(big_integer const& b)
{
	int8_t comparison = b.compare_by_abs(*this);
	if (comparison == 1)
	{
		this->data.clear();
		this->data.resize(1, 0);
		return *this;
	}
	else if (comparison == 0)
	{
		this->data.clear();
		this->data.resize(1, 1);
		return *this;
	}

	if (b.data.size() == 1)
	{
		if (this->data.size() == 1) //div_short_short
		{
			uint32_t div_res = this->data[0] / b.data[0];
			this->data.resize(1);
			this->data[0] = div_res;
			return *this;
		}
		else //div_long_short. <- makes tests 10 times faster.
		{
			this->get_remainder(b.data[0]);
			return *this;
		}
	}

	big_integer a(*this);
	this->data.clear();
	this->data.resize(a.data.size() - b.data.size() + 1);
	big_integer sub_res;

	big_integer tmp;
	tmp.data.resize(b.data.size());
	size_t x = a.data.size() - b.data.size();
	for (int32_t j = 0; j < b.data.size(); j++)//kind of range constructor
		tmp.data[j] = a.data[x + j];

	for (int32_t i = x; i >= 0; i--)
	{
		if (i != x) //if not first iteration
		{
			tmp = sub_res;
			tmp.data.resize(tmp.data.size() + 1);
			for (size_t i = tmp.data.size() - 1; i > 0; i--)
				tmp.data[i] = tmp.data[i - 1];

			tmp.data[0] = a.data[i];
			tmp.erase_leading_zeroes();
		}

		//binsearch to find out such k : b * k = tmp .
		// k >= 0 && k < BASE
		int64_t l = -1;
		int64_t r = BASE; //BASE is smaller, than MAX_INT64
		while (l < r - 1)
		{
			int64_t m = (l + r) / 2;
			big_integer b_copy(b);
			if ((b_copy.mul_long_short_unsigned(m)).compare_by_abs(tmp) != 1)
				l = m;
			else
				r = m;
		}
		this->data[i] = l;

		big_integer b_copy(b);
		sub_res = tmp.sub_long_long_unsigned(b_copy.mul_long_short_unsigned(l));
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
	for (size_t i = this->data.size(); i > 0; i--)
	{
		cur_sum = (uint64_t)this->data[i - 1] + carry * BASE;
		this->data[i - 1] = (size_t)(cur_sum / (uint64_t)other);
		carry = cur_sum % (uint64_t)other;
	}

	this->erase_leading_zeroes();
	return static_cast<uint32_t> (carry);
}

std::ostream &operator<<(std::ostream &s, big_integer const &a) {
	return s << to_string(a);
}