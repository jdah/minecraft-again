#ifndef UTIL_STRING_HPP
#define UTIL_STRING_HPP

#include <string>
#include <cctype>
#include <algorithm>

namespace util {

inline std::string to_lower(const std::string &str) {
    auto result = str;
    std::transform(
        result.begin(), result.end(), result.begin(),
        [](unsigned char c){ return std::tolower(c); });
    return result;
}

};

#endif
