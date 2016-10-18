#include <limits>
#include <string>
#include <iostream>

struct checked_exception
{
public:
    checked_exception(std::string const& m) : msg(m)
    { }
    std::string const& what() const
    {
        return msg;
    }
private:
    std::string msg;
};

struct signed_tag
{
};
struct unsigned_tag
{
};

template<typename T, bool is_signed>
struct tag_selector;

template<typename T>
struct tag_selector<T, true>
{
    typedef signed_tag tag;
};
template<typename T>
struct tag_selector<T, false>
{
    typedef unsigned_tag tag;
};

template<typename T>
struct checked_traits
{
    typedef typename tag_selector<T, std::numeric_limits<T>::is_signed>::tag tag;
};

//class "checked" is guaranteed to work correctly for integer types only.
template<typename T>
struct checked
{
public:
    checked()
    { }
    checked(T val) : value(val)
    { }

    checked<T>& operator=(checked<T> const& rhs)
    {
        this->value = rhs.value;
        return *this;
    }

    template<typename V>
    friend checked<V> operator+(checked<V> const&, checked<V> const&);
    template<typename V>
    friend checked<V> add_impl(checked<V> const&, checked<V> const&, signed_tag);
    template<typename V>
    friend checked<V> add_impl(checked<V> const&, checked<V> const&, unsigned_tag);

    template<typename V>
    friend checked<V> operator-(checked<V> const&, checked<V> const&);
    template<typename V>
    friend checked<V> sub_impl(checked<V> const&, checked<V> const&, signed_tag);
    template<typename V>
    friend checked<V> sub_impl(checked<V> const&, checked<V> const&, unsigned_tag);

    template<typename V>
    friend checked<V> operator*(checked<V> const&, checked<V> const&);
    template<typename V>
    friend checked<V> mul_impl(checked<V> const&, checked<V> const&, signed_tag);
    template<typename V>
    friend checked<V> mul_impl(checked<V> const&, checked<V> const&, unsigned_tag);

    template<typename V>
    friend checked<V> operator/(checked<V> const&, checked<V> const&);
    template<typename V>
    friend checked<V> div_impl(checked<V> const&, checked<V> const&, signed_tag);
    template<typename V>
    friend checked<V> div_impl(checked<V> const&, checked<V> const&, unsigned_tag);

    checked<T> unary_min_impl(signed_tag) const
    {
        if (this->value == std::numeric_limits<T>::min())
            throw checked_exception("unary minus signed overflow");
        return checked<T>(this->value * (T) (-1));
    }
    checked<T> unary_min_impl(unsigned_tag) const
    {
        throw checked_exception("unary minus is not defined for unsigned");
    }
    checked<T> operator-() const
    {
        return this->unary_min_impl(typename checked_traits<T>::tag());
    }

private:
    T value;
};

template<typename T>
checked<T> add_impl(checked<T> const& lhs, checked<T> const& rhs, signed_tag)
{
    //add for signed
    if (((rhs.value > 0) && (lhs.value > std::numeric_limits<T>::max()) - rhs.value) ||
        ((rhs.value < 0) && (lhs.value < std::numeric_limits<T>::min() - rhs.value)))
        throw checked_exception("add signed overflow\n");
    return checked<T>(lhs.value + rhs.value);
}
template<typename T>
checked<T> add_impl(checked<T> const& lhs, checked<T> const& rhs, unsigned_tag)
{
    //add for unsigned
    if (std::numeric_limits<T>::max() - lhs.value < rhs.value)
        throw checked_exception("add unsigned overflow\n");
    return checked<T>(lhs.value + rhs.value);
}

template<typename T>
checked<T> operator+(checked<T> const& lhs, checked<T> const& rhs)
{
    return add_impl(lhs, rhs, typename checked_traits<T>::tag());
}

template<typename T>
checked<T> sub_impl(checked<T> const& lhs, checked<T> const& rhs, signed_tag)
{
    //sub for signed
    if (((rhs.value < 0) && (lhs.value > std::numeric_limits<T>::max() + rhs.value)) ||
        ((rhs.value > 0) && (lhs.value < std::numeric_limits<T>::min() + rhs.value)))
        throw checked_exception("sub signed overflow\n");
    return checked<T>(lhs.value - rhs.value);
}
template<typename T>
checked<T> sub_impl(checked<T> const& lhs, checked<T> const& rhs, unsigned_tag)
{
    //sub for unsigned
    if (lhs.value < rhs.value)
        throw checked_exception("sub unsigned overflow\n");
    return checked<T>(lhs.value - rhs.value);
}

template<typename T>
checked<T> operator-(checked<T> const& lhs, checked<T> const& rhs)
{
    return sub_impl(lhs, rhs, typename checked_traits<T>::tag());
}

template<typename T>
checked<T> mul_impl(checked<T> const& lhs, checked<T> const& rhs, signed_tag)
{
    //mul for signed
    if (lhs.value != 0 && rhs.value != 0 &&
        ((std::abs(lhs.value) > std::numeric_limits<T>::max() / std::abs(rhs.value)) ||
         ((rhs.value == -1) && (lhs.value == std::numeric_limits<T>::min())) ||
         ((lhs.value == -1) && (rhs.value == std::numeric_limits<T>::min()))))
        throw checked_exception("mul signed overflow\n");
    return checked<T>(lhs.value * rhs.value);
}
template<typename T>
checked<T> mul_impl(checked<T> const& lhs, checked<T> const& rhs, unsigned_tag)
{
    //mul for unsigned
    if (lhs.value != 0 && rhs.value != 0 &&
        (lhs.value > std::numeric_limits<T>::max() / rhs.value))
        throw checked_exception("mul unsigned overflow\n");
    return checked<T>(lhs.value * rhs.value);
}

template<typename T>
checked<T> operator*(checked<T> const& lhs, checked<T> const& rhs)
{
    return mul_impl(lhs, rhs, typename checked_traits<T>::tag());
}

template<typename T>
checked<T> div_impl(checked<T> const& lhs, checked<T> const& rhs, signed_tag)
{
    //div for signed
    if ((rhs.value == -1) && (lhs.value == std::numeric_limits<T>::min()))
        throw checked_exception("div signed overflow\n");
    return checked<T>(lhs.value / rhs.value);
}
template<typename T>
checked<T> div_impl(checked<T> const& lhs, checked<T> const& rhs, unsigned_tag)
{
    //div for unsigned
    //it looks like overflow doesn't happen when dividing unsigned integer numbers 
    return checked<T>(lhs.value / rhs.value);
}

template<typename T>
checked<T> operator/(checked<T> const& lhs, checked<T> const& rhs)
{
    return div_impl(lhs, rhs, typename checked_traits<T>::tag());
}

int main()
{
    try
    {
        typedef int8_t type;

        {
            checked<type> v1(100);
            checked<type> v2(std::numeric_limits<type>::min());
            v1 + v2;
            v1 - v2;
            v1 * v2;
            v1 / v2;
            -v1;
            -v2;
        }
    }
    catch (checked_exception& e)
    {
        std::cout << e.what();
    }

    return 0;
}