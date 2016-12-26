//
// Created by Aleksandr Tukallo on 22.12.16.
//

#include <iostream>
#include "bind.h"

double func(int a, float b, int c, double d)
{
    std::cout << "f: " << a << " " << b << " " << c << " " << d << std::endl;
    return static_cast<double>(a + b + c + d);
}

void str_f(std::string strr)
{
    std::cout << "str_f: " << strr << std::endl;
}

struct my_type
{
    void operator()(int x, int y)
    {
        std::cout << "my_type::operator(): " << x << " " << y << std::endl;
    }
};

int twice(int a)
{
    return a * 2; 
}

void g(int a, int b)
{
    std::cout << "g " << a << " " << b << std::endl;
}

int main()
{
    //bind functions
    my_bind(func, 2, 2.3, std::placeholders::_1, 6.1)(100); //ok
    my_bind(&func, 2, 2.3, 100, 6.1)(); //ok, totally the same
    my_bind(str_f, "abs")("not using this at all"); //ok, not using new_args. Should be an error?
    my_bind(func, std::placeholders::_1, std::placeholders::_1, std::placeholders::_1, std::placeholders::_3)(1, -100, 3); //ok
    void (*ptr)(std::string) = str_f;
    my_bind(ptr, std::placeholders::_5)("1", "2", "3", "4", "5"); //explicit pointer to function usage
    my_bind(*******************ptr, std::placeholders::_1)("1", "2", "3", "4", "5"); //nice code

    std::cout << std::endl;
    //bind classes with operator() and std::function
    my_type obj;
    my_bind(obj, 12, -8)();
    my_bind(my_type(), 4, 3)();
    std::function<double(int, float, int, float)> functor = func;
    my_bind(functor, 1, std::placeholders::_2, 2, 3.4)(1, 12.6);

    std::cout << std::endl;
    //bind lambdas
    my_bind([](int a, int b){std::cout << "lambda " << a << " " << b << std::endl;}, 12, std::placeholders::_1)(125);
    int a = 5;
    auto lambda_auto = [a](int x){std::cout << "lambda " << a + x <<std::endl;};
    my_bind(lambda_auto, std::placeholders::_2)(1, 0);
    
    std::cout << std::endl;
    //bind bind
    my_bind(g, my_bind(twice, std::placeholders::_2), std::placeholders::_1)(2, 3);
    my_bind(g, my_bind(twice, std::placeholders::_1), my_bind(twice, std::placeholders::_1))(1);
    my_bind(g, my_bind(twice, std::placeholders::_1)(4), my_bind(twice, std::placeholders::_1))(1); //works correctly

    return 0;
}