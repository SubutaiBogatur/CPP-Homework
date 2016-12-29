#include "either.h"
#include <cassert>
 
template <typename Tag>
struct obj_count
{
    obj_count()
    {
        ++counter;
    }
 
    obj_count(int, int, int)
    {
        ++counter;
    }
   
    obj_count(obj_count const& other)
    {
        assert(&other != this);
        ++counter;
    }
   
    ~obj_count()
    {
        --counter;
    }
   
    static size_t counter;
};
 
struct copy_throws
{
    copy_throws()
    {}
   
    copy_throws(copy_throws&&)
    {}
 
    copy_throws(copy_throws const&)
    {
        throw 1;
    }
};
 
template <typename Tag>
size_t obj_count<Tag>::counter = 0;
 
struct foo_tag {};
struct bar_tag {};
 
typedef obj_count<foo_tag> foo;
typedef obj_count<bar_tag> bar;
 
template <typename T>
struct expect
{
    template <typename U>
    int operator()(U const&) const
    {
        assert(false);
        return 44;
    }
 
    int operator()(T const&) const
    {
        return 42;
    }
};
 
int main()
{
    {
        either<foo, bar> a(foo{});
        assert(a.is_left());
        assert(!a.is_right());
    }
 
    {
        either<foo, bar> a(bar{});
        assert(a.is_right());
        assert(!a.is_left());
    }
 
    {
        either<foo, bar> a(emplace_left, 1, 2, 3);
        assert(a.is_left());
        assert(!a.is_right());
    }
 
    {
        either<foo, bar> a(foo{});
        either<foo, bar> b = a;
    }
 
    {
        either<foo, bar> a(foo{});
        a = a;
        assert(a.is_left());
    }
 
    {
        either<foo, bar> a(foo{});
        either<foo, bar> b(bar{});
        a = b;
        a = b;
    }
 
    {
        either<foo, bar> a(foo{});
        either<foo, bar> b(bar{});
       
        apply(expect<foo>(), a);
        apply(expect<bar>(), b);
       
        swap(a, b);
 
        apply(expect<foo>(), b);
        apply(expect<bar>(), a);
    }
 
    {
        either<bar, copy_throws> a(bar{});
        either<bar, copy_throws> b(copy_throws{});
        assert(a.is_left());
        assert(b.is_right());
       
        try
        {
            a = b;
        }
        catch (int e)
        {
            assert(e == 1);
        }
        assert(a.is_left());
    }
 
    {
        either<bar, copy_throws> a(bar{});
        either<bar, copy_throws> b(copy_throws{});
        assert(a.is_left());
        assert(b.is_right());
       
        try
        {
            a = std::move(b);
            assert(false);
        }
        catch (int e)
        {
            assert(e == 1);
        }
        assert(a.is_left());
    }
   
    assert(foo::counter == 0);
    assert(bar::counter == 0);
}