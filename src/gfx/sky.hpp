#ifndef GFX_SKY_HPP
#define GFX_SKY_HPP

#include "util/util.hpp"

namespace gfx {
struct Sky {
    static const inline glm::vec4 COLORS[4][3] = {
        {
            util::color::u32_to_rgba(0x87CEEBFF),
            util::color::u32_to_rgba(0x87CEEBFF),
            util::color::u32_to_rgba(0x87CEEBFF)
        }, // DAY
        {
            util::color::u32_to_rgba(0x020206FF),
            util::color::u32_to_rgba(0x010104FF),
            util::color::u32_to_rgba(0x000000FF)
        }, // NIGHT
        {
            util::color::u32_to_rgba(0xFFCA7CFF),
            util::color::u32_to_rgba(0xFFCA7CFF),
            util::color::u32_to_rgba(0x000000FF)
        }, // SUNRISE
        {
            util::color::u32_to_rgba(0xFFAB30FF),
            util::color::u32_to_rgba(0xFFAB30FF),
            util::color::u32_to_rgba(0x000000FF)
        }  // SUNSET
    };
};
}

#endif
