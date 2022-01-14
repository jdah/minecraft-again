#ifndef GFX_SUN_HPP
#define GFX_SUN_HPP

#include "util/util.hpp"
#include "gfx/bgfx.hpp"
#include "gfx/program.hpp"

// forward declaration
namespace level {
    struct Area;
}

namespace gfx {
struct Sun {
    util::Camera camera;
    glm::vec3 direction;
    glm::vec3 diffuse, ambient;
    f32 texture_size;

    Sun() = default;
    Sun(f32 texture_size) : texture_size(texture_size) {}
    void update(
        level::Area &area,
        const util::PerspectiveCamera &camera);
    void set_uniforms(Program &program);
};
}

#endif
