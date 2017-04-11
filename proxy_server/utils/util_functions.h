//
// Created by Aleksandr Tukallo on 11.04.17.
//

#ifndef PROXY_SERVER_UTIL_FUNCTIONS_H
#define PROXY_SERVER_UTIL_FUNCTIONS_H

#define _GLIBCXX_USE_CXX11_ABI 0 //for nice gdb debugging

#include <functional>

namespace utils
{
    /**
     * \c ensure function is mainly needed for logging and monitoring current state of a server. It throws 
     * \c server_exception if \c if_successful function returns \c false. If it returns \c true,
     * \c on_success string is printed in \c std::cerr.
     * @param val is usually a return result of syscall we want to check
     * @param if_successful function, that returns true if \c val is ok, \c false otherwise
     * @param on_success is string logged if \c val is checked and everything is ok.
     */
    void ensure(int val, std::function<bool(int)> if_successful, std::string on_success);

    /**
     * This overloaded version just prints the string to \c std::cerr
     * @param on_success string to be printed
     */
    void ensure(std::string on_success);

    //here are some functions, that can be passed to \c ensure function
    bool is_not_negative(int a);
    bool is_zero(int a);
}


#endif //PROXY_SERVER_UTIL_FUNCTIONS_H
