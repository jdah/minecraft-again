#ifndef UTIL_COLOR_HPP
#define UTIL_COLOR_HPP

#include "util/types.hpp"
#include "util/math.hpp"

namespace util {
namespace color {
    inline glm::vec4 u32_to_rgba(u32 c) {
        return glm::vec4(
            ((c >> 24) & 0xFF) / 255.0f,
            ((c >> 16) & 0xFF) / 255.0f,
            ((c >>  8) & 0xFF) / 255.0f,
            ((c >>  0) & 0xFF) / 255.0f);
    }
};
};

#endif
