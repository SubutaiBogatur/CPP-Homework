//
// Created by Aleksandr Tukallo on 29.12.16.
//

#ifndef EITHER_EITHER_H
#define EITHER_EITHER_H

#include <type_traits>
#include <algorithm>
#include <memory>
#include <bits/stdc++.h>


struct emplace_left_t
{
};
static emplace_left_t emplace_left;

struct emplace_right_t
{
};
static emplace_right_t emplace_right;

enum object_stored
{
    LEFT,
    RIGHT,
    LEFT_PTR,
    RIGHT_PTR
};

template<typename Left, typename Right>
struct either
{
    //inv stores one from two always
    //inv Left != Right

    //thus no dflt constructor
    either(Left); //value, because moving
    either(Right); //same reason
    either(either const&);
    either(either&&);

    ~either();

    either& operator=(either); //inside always move. If rvalue calling side nothing makes

    //accessing
    bool is_left() const;
    Left& left();
    Left const& left() const; //if Right, UB

    bool is_right() const;
    Right& right();
    Right const& right() const;

    template<typename... Args>
    void
    emplace(emplace_left_t, Args&& ... args); //moves either to left condition, left is created with this constructor

    template<typename... Args>
    void emplace(emplace_right_t, Args&& ... args);

    template<typename... Args>
    either(emplace_left_t, Args&&... args);

    template<typename... Args>
    either(emplace_right_t, Args&&... args);

    //eg either<int, x> a(emplace_right, 1, 2, 3);

    //todo make private
private:
    either() = delete;

    void call_current_destructor();

    std::unique_ptr<Left>* get_left_ptr_ptr();
   // std::unique_ptr<Left> const* get_left_ptr_ptr();

    object_stored which; 

    typename std::aligned_storage<std::max({sizeof(Left), sizeof(Right), sizeof(Left*)}), 
                                  std::max({alignof(Left), alignof(Right), alignof(Left*)})>::type data;     

    template<typename NewType, typename CurType, typename... Args>
    void emplace_template (CurType old, object_stored newType, Args&&... args);     

    template<typename FLeft, typename FRight, typename Rlhs, typename Rrhs> 
    friend void swap_impl(either<FLeft, FRight> &lhs, either<FLeft, FRight> &rhs,
                        std::unique_ptr<Rlhs> ldata, std::unique_ptr<Rrhs> rdata,
                        object_stored const& ltype, object_stored const& rtype);                    
};


template<typename F, typename Left, typename Right>
auto apply(F const&, either<Left, Right> const&); //one more overloading needed, same but either not const
//calls F from Left if left, or Right

//for swap also strong guarantee, same contract as for emplace and operator=

template<typename Left, typename Right>
void swap(either<Left, Right>&, either<Left, Right>&);

//____________________________________________________________
//implementation:

template<typename Left, typename Right>
either<Left, Right>::either(Left left)
{
    which = LEFT;
    new(&data)Left(std::move(left));
}

template<typename Left, typename Right>
either<Left, Right>::either(Right right)
{
    which = RIGHT;
    new(&data)Right(std::move(right));
}

template<typename Left, typename Right>
either<Left,Right>::either(either<Left,Right> const& other)
{   
    this->which = other.which;
    switch(which)
    {
        case LEFT:
            new(&data)Left(std::forward<const Left&>(other.left()));
            break;
        case RIGHT:
            new(&data)Right(std::forward<const Right&>(other.right()));
            break;
        case LEFT_PTR:
            new(&data)Left(std::forward<const Left&>(other.left()));
            which = LEFT;
            break;
        case RIGHT_PTR:
            new(&data)Right(std::forward<const Right&>(other.right()));
            which = RIGHT; 
            break;
    }
}

template<typename Left, typename Right>
either<Left, Right>::either(either<Left, Right>&& other)
{
    if (other.is_left()) {
        which = LEFT;
        emplace(emplace_left, other.left());
    }
    else {
        which = RIGHT;
        emplace(emplace_right, other.right());
    }
}

template<typename Left, typename Right>
either<Left, Right>& either<Left, Right>::operator=(either other)
{
    if (other.is_left())
        emplace(emplace_left, other.left());
    else 
        emplace(emplace_right, other.right());
    return *this;   
} 

template<typename Left, typename Right>
either<Left,Right>::~either()
{
    call_current_destructor();
}

template<typename Left, typename Right>
bool either<Left,Right>::is_left() const
{
    return which == LEFT || which == LEFT_PTR;
}

template<typename Left, typename Right>
bool either<Left,Right>::is_right() const
{
    return !is_left();
}

template<typename Left, typename Right>
Left& either<Left, Right>::left()
{
    assert(which == LEFT_PTR || which == LEFT);

    if(which == LEFT)
        return *reinterpret_cast<Left*>(&this->data);
    //else LEFT_PTR
    return **reinterpret_cast<std::unique_ptr<Left>*>(&this->data);
}

template<typename Left, typename Right>
Left const& either<Left, Right>::left() const
{
    assert(which == LEFT_PTR || which == LEFT);

    if(which == LEFT)
        return *reinterpret_cast<Left const*>(&this->data);
    //else LEFT_PTR
    return **reinterpret_cast<std::unique_ptr<Left> const*>(&this->data);
}


template<typename Left, typename Right>
Right& either<Left, Right>::right()
{
    assert(which == RIGHT_PTR || which == RIGHT);

    if(which == RIGHT)
        return *reinterpret_cast<Right*>(&this->data);
    //else LEFT_PTR
    return **reinterpret_cast<std::unique_ptr<Right>*>(&this->data);
}


template<typename Left, typename Right>
Right const& either<Left, Right>::right() const
{
    assert(which == RIGHT_PTR || which == RIGHT);

    if(which == RIGHT)
        return *reinterpret_cast<Right const*>(&this->data);
    //else LEFT_PTR
    return **reinterpret_cast<std::unique_ptr<Right> const*>(&this->data);
}


template<typename Left, typename Right>
void either<Left, Right>::call_current_destructor()
{
    switch(which)
    {
        case LEFT:
            reinterpret_cast<Left*>(&this->data)->Left::~Left();
            break;
        case RIGHT:
            reinterpret_cast<Right*>(&this->data)->Right::~Right();
            break;
        case LEFT_PTR:
            reinterpret_cast<std::unique_ptr<Left>*>(&this->data)->std::unique_ptr<Left>::~unique_ptr<Left>();
        case RIGHT_PTR:
            reinterpret_cast<std::unique_ptr<Right>*>(&this->data)->std::unique_ptr<Right>::~unique_ptr<Right>();
    }
}

template<typename Left, typename Right>
template<typename NewType, typename CurType, typename... Args>
void either<Left, Right>::emplace_template(CurType old, object_stored newType, Args&&... args)
{
    //copy current value to heap
    std::unique_ptr<CurType> tmp = std::make_unique<CurType>(old);

    //destroy value in data
    call_current_destructor();
    try
    {
        //trying to call constructor of new data in storage
        new (&data) NewType(std::forward<Args> (args)...);                    
        which = newType; //if success, everything ok
        tmp.reset();
    }
    catch (...)
    {
        //switch to case when in heap
        if(newType == LEFT)
            which = LEFT_PTR;
        if(newType == RIGHT)
            which = RIGHT_PTR;

        //create ptr in data 
        new (&data) std::unique_ptr<CurType>(tmp.release());
        throw;
    }   
}                 

template<typename Left, typename Right>
template<typename... Args>
void either<Left, Right>::emplace(emplace_left_t, Args&&... args)
{
    if (which == LEFT)
        emplace_template<Left>(left(), LEFT, args...);
    else 
        emplace_template<Left>(right(), LEFT, args...);              
}

template<typename Left, typename Right>
template<typename... Args>
void either<Left, Right>::emplace(emplace_right_t, Args&&... args)
{
    if (which == LEFT)
        emplace_template<Right>(left(), RIGHT, args...);
    else 
        emplace_template<Right>(right(), RIGHT, args...);              
}

template<typename Left, typename Right>
template<typename... Args>
either<Left,Right>::either(emplace_left_t, Args&&... args) 
{
    which = LEFT;
    new (&data) Left(std::forward<Args>(args)...);
}

template<typename Left, typename Right>
template<typename... Args>
either<Left,Right>::either(emplace_right_t, Args&&... args) 
{
    which = RIGHT;
    new (&data) Right(std::forward<Args>(args)...);
}

template<typename F, typename Left, typename Right>
auto apply(F const& f, either<Left, Right> const& cur)
{
    if (cur.is_left()) 
        return f(cur.left());
    else 
        return f(cur.right());          
}

template<typename F, typename Left, typename Right>
auto apply(F const& f, either<Left, Right>& cur)
{
    if (cur.is_left())
        return f(cur.left());
    else
        return f(cur.right());          
}

template<typename Left, typename Right, typename Rlhs, typename Rrhs> 
void swap_impl(either<Left, Right> &lhs, either<Left, Right> &rhs,
        std::unique_ptr<Rlhs> ldata, std::unique_ptr<Rrhs> rdata,
        object_stored const& ltype, object_stored const& rtype) {

        try
        {
            lhs.call_current_destructor();
            new (&lhs.data) Rrhs(*rdata);
            lhs.which = rtype;

            rhs.call_current_destructor();
            new (&rhs.data) Rlhs(*ldata);
            rhs.which = ltype;
        }
        catch (...)
        {
            if(lhs.which == LEFT)
                lhs.which = LEFT_PTR;
            if(lhs.which == RIGHT)
                lhs.which = RIGHT_PTR;

            if(rhs.which == LEFT)
                rhs.which = LEFT_PTR;
            if(rhs.which == RIGHT)
                rhs.which = RIGHT_PTR;

            new (&lhs.data) std::unique_ptr<Rlhs>(ldata.release());
            new (&rhs.data) std::unique_ptr<Rrhs>(rdata.release());
            throw;
        }
    }

template<typename Left, typename Right> 
void swap(either<Left, Right> &lhs, either<Left, Right> &rhs)
{
    if (lhs.is_left() && rhs.is_left())
        swap_impl(lhs, rhs, std::make_unique<Left>(lhs.left()), std::make_unique<Left>(rhs.left()), LEFT, LEFT);
    else if (lhs.is_right() && rhs.is_left())
        swap_impl(lhs, rhs, std::make_unique<Right>(lhs.right()), std::make_unique<Left>(rhs.left()), RIGHT, LEFT);
    else if (lhs.is_left() && rhs.is_right())
        swap_impl(lhs, rhs, std::unique_ptr<Left>(std::make_unique<Left>(lhs.left())), std::unique_ptr<Right>(std::make_unique<Right>(rhs.right())), LEFT, RIGHT);
    else
        swap_impl(lhs, rhs, std::make_unique<Right>(lhs.right()), std::make_unique<Right>(rhs.right()), RIGHT, RIGHT);
}

#endif //EITHER_EITHER_H
