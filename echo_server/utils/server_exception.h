//
// Created by Aleksandr Tukallo on 11.04.17.
//

#ifndef PROXY_SERVER_SERVER_EXCEPTION_H
#define PROXY_SERVER_SERVER_EXCEPTION_H

#define _GLIBCXX_USE_CXX11_ABI 0 //for nice gdb debugging

#include <exception>
#include <string>

/**
 * @brief This exception is thrown, when an error or an unexpected situation during server runtime has happened
 */
struct server_exception : std::exception
{
public:
    //for lvalue, copying is made inside
    server_exception(const std::string& msg) : msg(msg)
    {}

    //for rvalue, no copying at all
    server_exception(std::string&& msg) : msg(std::move(msg))
    {}

    const char *what() const noexcept override
    {
        return msg.c_str();
    }

    std::string get_msg()
    {
        return msg;
    }

private:
    std::string msg;
};


#endif //PROXY_SERVER_SERVER_EXCEPTION_H
