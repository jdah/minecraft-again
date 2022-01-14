#ifndef UTIL_MATH_HPP
#define UTIL_MATH_HPP

#include <cstdint>
#include <limits>
#include <random>
#include <type_traits>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_SWIZZLE
#define GLM_ENABLE_EXPERIMENTAL

// #define GLM_FORCE_LEFT_HANDED

#include "util/std.hpp"
#include "util/types.hpp"

// TODO: !!! VERY IMPORTANT !!! INCLUDE OTHER PLATFORMS WITH [0, 1] NDC HERE
#ifdef SHADER_TARGET_metal
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/hash.hpp>

namespace util {
// TEMPLATE UTILITIES
// glm::vec
template <class T>
struct is_vec {
    static const bool value = false;
};

template <glm::length_t L, class T, glm::qualifier Q>
struct is_vec<glm::vec<L, T, Q>> {
    static const bool value = true;
};

// glm::mat
template <class T>
struct is_mat {
    static const bool value = false;
};

template <glm::length_t C, glm::length_t R, class T, glm::qualifier Q>
struct is_mat<glm::mat<C, R, T, Q>> {
    static const bool value = true;
};

// custom glm extensions
template <
    glm::length_t L,
    class T,
    glm::qualifier Q>
inline bool nonzero(const glm::vec<L, T, Q> &v) {
    return
        glm::any(
            glm::epsilonNotEqual(v, glm::vec<L, T, Q>(0), glm::epsilon<T>()));
}

// MISC UTILITIES

// find the smallest possible t such that s + t * ds is an integer
inline glm::vec3 intbound(glm::vec3 s, glm::vec3 ds) {
    glm::vec3 res;
    UNROLL(3)
    for (usize i = 0; i < 3; i++) {
        res[i] =
            (ds[i] > 0 ?
                (glm::ceil(s[i]) - s[i])
                : (s[i] - glm::floor(s[i])))
            / glm::abs(ds[i]);
    }
    return res;
}

struct Rand {
    std::mt19937_64 rng;
    std::uniform_int_distribution<usize> d_int;
    std::uniform_real_distribution<f64> d_real;
    std::normal_distribution<f64> d_normal;

    Rand(std::optional<usize> seed)
        : rng(std::random_device()()),
          d_int(0, UINT64_MAX),
          d_real(0.0, 1.0),
          d_normal(0.0, 1.0) {
        if (seed) {
            this->seed(*seed);
        }
    }

    // NOTE: bounds are inclusive
    template <typename T>
        requires std::is_integral<T>::value
    inline T next(
        T min = std::numeric_limits<T>::min(),
        T max = std::numeric_limits<T>::max()) {
        return (static_cast<T>(this->d_int(rng)) % (max - min + 1)) + min;
    }

    template <typename T>
        requires std::is_floating_point<T>::value
    inline T next(
        T min = std::numeric_limits<T>::min(),
        T max = std::numeric_limits<T>::max()) {
        return glm::mod(static_cast<T>(this->d_real(rng)), max - min) + min;
    }

    inline f64 gaussian() {
        return d_normal(rng);
    }

    inline void seed(usize s) {
        rng.seed(s);
    }
};

// random number generator with specified seed
inline auto rand(std::optional<usize> seed) {
    return Rand(seed);
}

// random number generator with seed based on hash of object
template <typename T>
inline auto rand_from_hash(T t) {
    return Rand(std::hash<T>{}(t));
}
}


#endif
