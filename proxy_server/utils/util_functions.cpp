//
// Created by Aleksandr Tukallo on 11.04.17.
//

#include "util_functions.h"
#include <cstring>
#include <iostream>
#include "server_exception.h"

bool ::utils::is_not_negative(int a)
{
    return a >= 0;
}
bool ::utils::is_zero(int a)
{
    return a == 0;
}

void ::utils::ensure(int val, std::function<bool(int)> if_successful, std::string on_success)
{
    if (!if_successful(val))
    {
        throw server_exception("Error is" + std::string(strerror(errno)) + ". ");
    }
    std::cerr << on_success;
}
