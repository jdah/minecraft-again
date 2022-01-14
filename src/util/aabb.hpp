#ifndef UTIL_AABB_HPP
#define UTIL_AABB_HPP

#include "glm/fwd.hpp"
#include "util/types.hpp"
#include "util/std.hpp"
#include "util/math.hpp"

namespace util {
template <typename T>
struct _AABB {
    using V = glm::vec<3, T, glm::defaultp>;
    using M = glm::mat<4, 4, T, glm::defaultp>;

    V min, max;

    _AABB() = default;
    _AABB(V _max) : min(0), max(_max) {}
    _AABB(V _min, V _max) : min(_min), max(_max) {}

    static inline _AABB<T> unit() {
        return _AABB(V(1));
    }

    inline _AABB<T> transform(const M &m) const {
        return _AABB<T>(m * this->min, m * this->max);
    }

    inline _AABB<T> translate(const V &v) const {
        return _AABB<T>(this->min + v, this->max + v);
    }

    // scales, keeping min in place
    inline _AABB<T> scale(const V &v) const {
        const auto d = this->max - this->min;
        return _AABB<T>(this->min, d * v);
    }

    // scales, keeping min in place
    inline _AABB<T> scale(const T &s) const {
        return scale(V(s));
    }

    // scales, keeping center in place
    inline _AABB<T> scale_center(const V &v) const {
        const auto
            c = (this->min + this->max) / static_cast<T>(2),
            d = this->max - this->min,
            h = d / static_cast<T>(2),
            e = h * v;
        return _AABB<T>(c - e, c + e);
    }

    // scales, keeping center in place
    inline _AABB<T> scale_center(const T &s) const {
        return scale_center(V(s));
    }

    // center on the specified point
    // can optionally only center on some axes
    inline _AABB<T> center_on(
        const V &v, glm::bvec3 axes = glm::bvec3(1)) const {
        const auto
            d = this->max - this->min,
            h = d / static_cast<T>(2);

        return _AABB<T>(
            glm::mix(this->min, v - h, glm::vec3(axes)),
            glm::mix(this->max, v + h, glm::vec3(axes)));
    }

    // calculate the center
    inline V center() const {
        return (this->min + this->max) / static_cast<T>(2);
    }

    // calculate the size
    inline V size() const {
        return this->max - this->min;
    }

    // calculates the collision depth of this AABB into other
    inline V depth(const _AABB<T> &other) const {
        const auto &a = *this, &b = other;
        V res,
          c_a = a.center(),
          c_b = b.center();

        #pragma unroll
        for (usize i = 0; i < 3; i++) {
            res[i] =
                c_a[i] < c_b[i] ?
                    a.max[i] - b.min[i]
                    : b.max[i] - a.min[i];
        }

        return res;
    }

    // returns true if this AABB collides with the other
    inline bool collides(const _AABB<T> &other) const {
        return
            this->min.x <= other.max.x &&
            this->max.x >= other.min.x &&
            this->min.y <= other.max.y &&
            this->max.y >= other.min.y &&
            this->min.z <= other.max.z &&
            this->max.z >= other.min.z;
    }

    // returns true if this AABB contains the specified point
    inline bool contains(const V &point) const {
        return
            point.x >= this->min.x &&
            point.x <= this->max.x &&
            point.y >= this->min.y &&
            point.y <= this->max.y &&
            point.z >= this->min.z &&
            point.z <= this->max.z;
    }

    inline std::string to_string() const {
        return
            "AABB(" + glm::to_string(this->min) + ", " +
                      glm::to_string(this->max) + ")";
    }
};

typedef _AABB<f32> AABB;
typedef _AABB<int> AABBi;

};

#endif
