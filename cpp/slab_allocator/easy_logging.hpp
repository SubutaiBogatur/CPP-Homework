//
// Created by atukallo on 3/20/19.
//

#include <iostream>
#include <sstream>

#ifndef CSC_HW_EASY_LOGGING_HPP

namespace logging {

    struct base_elogger {
        base_elogger() : string_stream() {}

        base_elogger(std::string str) : string_stream(str, std::ios_base::ate) {}

        template<class T>
        base_elogger &operator<<(const T &arg) {
#ifdef LOGGING
            string_stream << arg;
            return *this;
        }

        ~base_elogger() {
            std::string output = string_stream.str();
            std::cout << output << std::endl;
#endif
        }

    private:
        std::ostringstream string_stream;
    };

    struct error : base_elogger {
        error() : base_elogger("ERROR: ") {}
    };

    struct warn : base_elogger {
        warn() : base_elogger("WARN: ") {}
    };

    struct info : base_elogger {
        info() : base_elogger("INFO: ") {}
    };

    struct debug : base_elogger {
        debug() : base_elogger("DEBUG: ") {}
    };

}

#define CSC_HW_EASY_LOGGING_HPP

#endif //CSC_HW_EASY_LOGGING_HPP
