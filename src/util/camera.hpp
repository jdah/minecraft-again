#ifndef UTIL_CAMERA_HPP
#define UTIL_CAMERA_HPP

#include "gfx/bgfx.hpp"
#include "util/types.hpp"
#include "util/math.hpp"

// forward declaration
namespace gfx {
    struct Program;
}

namespace util {
struct Camera {
    glm::mat4 view, proj;

    Camera() = default;

    inline void set_view_transform(bgfx::ViewId _view = 0) {
        bgfx::setViewTransform(_view, &view, &proj);
    }

    virtual void set_uniforms(const std::string &prefix, gfx::Program &p);

    virtual void update(const glm::mat4 &view = glm::mat4(0));
};

struct OrthoCamera : Camera {
    glm::vec2 min, max, depth_range, position = glm::vec2(0);

    OrthoCamera() = default;
    OrthoCamera(glm::vec2 min, glm::vec2 max, glm::vec2 depth_range);
    void update(const glm::mat4 &view = glm::mat4(0)) override;
};

struct PerspectiveCamera : Camera {
    glm::vec3 position, direction, up, right;
    f32 pitch, yaw, fov, aspect;
    glm::vec2 depth_range;

    PerspectiveCamera() = default;
    PerspectiveCamera(
        f32 fov, f32 aspect, glm::vec2 depth_range,
        glm::vec3 position = glm::vec3(0.0f));
    void update(const glm::mat4 &view = glm::mat4(0)) override;
    void set_uniforms(const std::string &prefix, gfx::Program &p) override;
};
}

#endif
