#ifndef UTIL_NOISE_HPP
#define UTIL_NOISE_HPP

#include "util/types.hpp"
#include "util/math.hpp"

namespace util {
    struct Noise {
        virtual f32 sample(glm::vec2 i) const = 0;
    };

    struct Octave : Noise {
        u64 seed;
        usize n;
        f32 o;

        Octave(u64 seed, usize n, f32 o) : seed(seed), n(n), o(o) {}
        f32 sample(glm::vec2 i) const override;
    };

    struct Combined : Noise {
        Noise *n, *m;

        Combined(Noise &n, Noise &m) : n(&n), m(&m) {}
        f32 sample(glm::vec2 i) const override;
    };
};

#endif
