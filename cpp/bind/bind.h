//
// Created by Aleksandr Tukallo on 22.12.16.
//

//Compile with -std=c++14 flag because of auto returned from function

#ifndef BIND_H

#define BIND_H

#include <tuple>
#include <functional>

template<typename F, typename... Old_Args>
struct my_bind_type
{
    template<typename F_, typename... Old_Args_>
    friend my_bind_type<F_, Old_Args_...> my_bind(F_&&, Old_Args_&& ...);

public:
    template<typename... New_Args>
    auto operator()(New_Args&& ... new_args)
    {
        return call(typename seq_generator<std::tuple_size<tuple_t>::value>::type(),
                    std::forward<New_Args>(new_args)...);
    }

private:
    typedef F func_t; //universal reference 
    typedef std::tuple<Old_Args...> tuple_t;

    func_t f;
    tuple_t tuple;

    my_bind_type(F&& f, std::tuple<Old_Args...>&& tuple) : f(std::forward<F>(f)),
                                                           tuple(std::forward<tuple_t>(tuple))
    {}

    //sequence classes
    template<size_t... Indexes>
    struct sequence_of_size_t
    {
    };

    template<size_t Head, size_t... Indexes>
    struct seq_generator
    {
        typedef typename seq_generator<Head - 1, Head - 1, Indexes...>::type type;
    };

    template<size_t... Indexes>
    struct seq_generator<0, Indexes...>
    {
        typedef sequence_of_size_t<Indexes...> type;
    };


    //choosing correct arguments from old\new

    //usual value
    template<typename Old_Arg, typename... New_Args>
    auto&& arg_get(Old_Arg&& old_arg, New_Args&& ...) const
    {
        return old_arg;
    }

    //if old_arg is placeholder
    template<int N, typename... New_Args>
    auto&& arg_get(std::_Placeholder<N>, New_Args&& ... new_args) const
    {
        return std::get<N - 1>(std::forward_as_tuple(new_args...));
    }

    //old_arg is also a my_bind
    template<typename New_Function, typename... Another_Args, typename... New_Args>
    auto arg_get(my_bind_type<New_Function, Another_Args...>& another_bind, New_Args&& ... new_args) const
    {
        return another_bind(std::forward<New_Args>(new_args)...);
    }


    template<size_t... Indexes, typename... New_Args>
    auto call(sequence_of_size_t<Indexes...>, New_Args&& ... new_args)
    {
        return f(arg_get(std::get<Indexes>(tuple),
                         new_args...)...); //not forwarding because then new_args can be moved somewhere
        //return f(arg_get(std::get<Indexes>(tuple), std::forward<New_Args>(new_args)...)...);
    }

};


template<typename F, typename... Old_Args>
my_bind_type<F, Old_Args...> my_bind(F&& f, Old_Args&& ... args) //universal references onlyyy
{
    return my_bind_type<F, Old_Args...>(std::forward<F>(f),
                                        std::tuple<Old_Args...>(std::forward<Old_Args>(args)...)); //tuple constructor here doesn't copy anything, because arguments are universal refs
}

#endif //BIND_H