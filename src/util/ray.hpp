#ifndef UTIL_RAY_HPP
#define UTIL_RAY_HPP

#include "glm/gtx/norm.hpp"
#include "util/types.hpp"
#include "util/math.hpp"
#include "util/aabb.hpp"
#include "util/direction.hpp"

namespace util {
struct Ray {
    glm::vec3 origin, direction;

    Ray() = default;
    Ray(const glm::vec3& o, const glm::vec3 &d)
        : origin(o), direction(d) {}

    inline std::optional<std::tuple<glm::ivec3, util::Direction>>
        intersect_block(std::function<bool(glm::ivec3)> f, f32 max_distance) {
        util::Direction d = 0;
        glm::ivec3 p, step;
        glm::vec3 t_max, t_delta;
        f32 radius;

        p = glm::floor(this->origin);
        step = glm::sign(this->direction);
        t_max = util::intbound(this->origin, this->direction);
        t_delta = glm::vec3(step) / this->direction;
        radius = max_distance / glm::l2Norm(this->direction);

        while (true) {
            if (f(p)) {
                return std::make_optional(std::make_tuple(p, d));
            }

            if (t_max.x < t_max.y) {
                if (t_max.x < t_max.z) {
                    if (t_max.x > radius) {
                        break;
                    }

                    p.x += step.x;
                    t_max.x += t_delta.x;
                    d = glm::ivec3(-step.x, 0, 0);
                } else {
                    if (t_max.z > radius) {
                        break;
                    }

                    p.z += step.z;
                    t_max.z += t_delta.z;
                    d = glm::ivec3(0, 0, -step.z);
                }
            } else {
                if (t_max.y < t_max.z) {
                    if (t_max.y > radius) {
                        break;
                    }

                    p.y += step.y;
                    t_max.y += t_delta.y;
                    d = glm::ivec3(0, -step.y, 0);
                } else {
                    if (t_max.z > radius) {
                        break;
                    }

                    p.z += step.z;
                    t_max.z += t_delta.z;
                    d = glm::ivec3(0, 0, -step.z);
                }
            }
        }

        return std::nullopt;
    }
};
}

#endif
