#ifndef GFX_DEBUG_HPP
#define GFX_DEBUG_HPP

#include "bgfx/bgfx.h"
#include "util/util.hpp"
#include "gfx/bgfx.hpp"
#include "gfx/program.hpp"

namespace gfx {
struct Primitive {
    bgfx::VertexLayout layout_color, layout_texture, layout_color_texture;

    Primitive();
    void render_quad(
        glm::vec3 min, glm::vec3 max, glm::vec2 st_min, glm::vec2 st_max,
        const std::function<void(void)> &setup,
        const Program &program,
        bool invert_t = false, bgfx::ViewId view = 0, u64 render_state = 0);
    void render_aabb(
        util::AABB aabb, glm::vec4 color,
        bgfx::ViewId view = 0, u64 render_state = 0);
};
}

#endif
