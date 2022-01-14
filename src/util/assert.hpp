#ifndef UTIL_ASSERT_HPP
#define UTIL_ASSERT_HPP

#include "util/log.hpp"

namespace util {
    static inline void _assert(
        bool e,
        const std::string &message = "",
        bool critical = true,
        const char *_file = __builtin_FILE(),
        const char *_caller = __builtin_FUNCTION(),
        const usize _line = __builtin_LINE()) {
        if (!e) {
            util::log::print(
                "ASSERTION FAILED"
                    + (message.size() > 0 ? (": " + message) : ""),
                util::log::ERROR,
                _file, _caller, _line);

            if (critical) {
                std::exit(1);
            }
        }
    }
}

#endif
