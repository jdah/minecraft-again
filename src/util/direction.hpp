#ifndef UTIL_DIRECTION_HPP
#define UTIL_DIRECTION_HPP

#include <compare>

#include "util/std.hpp"
#include "util/types.hpp"
#include "util/math.hpp"

namespace util {
struct Direction {
    usize value;

    Direction(usize value) : value(value) {}
    Direction(glm::ivec3 v) : value(from_ivec3(v).value()) {}

    static inline const usize
        SOUTH  = 0,         // +z
        NORTH  = 1,         // -z
        EAST   = 2,         // +x
        WEST   = 3,         // -x
        TOP    = 4,         // +y
        BOTTOM = 5,         // -y
        COUNT  = 6,
        UP     = TOP,
        DOWN   = BOTTOM;

    static inline const constexpr auto ALL =
        { SOUTH, NORTH, EAST, WEST, TOP, BOTTOM };

    inline auto operator<=>(const Direction &other) const = default;

    inline auto operator<=>(const usize &other) const {
        return this->value <=> other;
    }

    inline Direction operator+(const Direction &other) const {
        return this->value + other.value;
    }

    inline Direction &operator++() {
        this->value++;
        return *this;
    }

    inline Direction operator++(int) {
        Direction d = *this;
        this->value++;
        return d;
    }

    inline operator usize() const {
        return this->value;
    }

    inline operator glm::ivec3() const {
        return to_ivec3(*this);
    }

    inline operator glm::vec3() const {
        return to_vec3(*this);
    }

    static inline glm::ivec3 to_ivec3(const Direction &d) {
        return ((glm::ivec3 []) {
            glm::ivec3( 0,  0,  1),
            glm::ivec3( 0,  0, -1),
            glm::ivec3( 1,  0,  0),
            glm::ivec3(-1,  0,  0),
            glm::ivec3( 0,  1,  0),
            glm::ivec3( 0, -1,  0)})[d];
    }

    static inline std::optional<Direction> from_ivec3(const glm::ivec3 &v) {
        if      (v == glm::ivec3( 0,  0,  1)) { return SOUTH; }
        else if (v == glm::ivec3( 0,  0, -1)) { return NORTH; }
        else if (v == glm::ivec3( 1,  0,  0)) { return EAST; }
        else if (v == glm::ivec3(-1,  0,  0)) { return WEST; }
        else if (v == glm::ivec3( 0,  1,  0)) { return TOP; }
        else if (v == glm::ivec3( 0, -1,  0)) { return BOTTOM; }
        else { return std::nullopt; }
    }

    static inline glm::vec3 to_vec3(const Direction &d) {
        return ((glm::vec3 []) {
            glm::vec3( 0,  0,  1),
            glm::vec3( 0,  0, -1),
            glm::vec3( 1,  0,  0),
            glm::vec3(-1,  0,  0),
            glm::vec3( 0,  1,  0),
            glm::vec3( 0, -1,  0)})[d];
    }

    static inline std::optional<Direction> from_vec3(const glm::vec3 &v) {
        if (glm::all(glm::epsilonEqual(v, glm::vec3( 0,  0,  1), glm::epsilon<f32>())))  { return SOUTH; }
        else if (glm::all(glm::epsilonEqual(v, glm::vec3( 0,  0, -1), glm::epsilon<f32>())))  { return NORTH; }
        else if (glm::all(glm::epsilonEqual(v, glm::vec3( 1,  0,  0), glm::epsilon<f32>())))  { return EAST; }
        else if (glm::all(glm::epsilonEqual(v, glm::vec3(-1,  0,  0), glm::epsilon<f32>())))  { return WEST; }
        else if (glm::all(glm::epsilonEqual(v, glm::vec3( 0,  1,  0), glm::epsilon<f32>())))  { return TOP; }
        else if (glm::all(glm::epsilonEqual(v, glm::vec3( 0, -1,  0), glm::epsilon<f32>())))  { return BOTTOM; }
        else { return std::nullopt; }
    }
};
}

#endif
