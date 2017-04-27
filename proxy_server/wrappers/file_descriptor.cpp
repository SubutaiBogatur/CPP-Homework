//
// Created by Aleksandr Tukallo on 27.04.17.
//

#include "file_descriptor.h"
#include "../utils/util_functions.h"
#include <unistd.h>

file_descriptor::~file_descriptor()
{
    utils::ensure(close(fd_val), utils::is_zero,
                  "File descriptor " + std::to_string(fd_val) + " was closed\n");
}

file_descriptor::file_descriptor(int val) : fd_val(val)
{
    utils::ensure(val, utils::is_not_negative,
                  "Fd " + std::to_string(val) + " wrapped\n");
}

int file_descriptor::get_fd()
{
    return fd_val;
}
