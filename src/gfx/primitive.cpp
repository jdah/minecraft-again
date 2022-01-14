#include "gfx/primitive.hpp"
#include "bgfx/bgfx.h"
#include "bgfx/defines.h"
#include "state.hpp"

using namespace gfx;

struct VertexColor {
    glm::vec3 position;
    glm::vec4 color;

    VertexColor() = default;
    VertexColor(glm::vec3 position, glm::vec4 color)
        : position(position), color(color) {}
};

struct VertexTexture {
    glm::vec3 position;
    glm::vec2 st;

    VertexTexture() = default;
    VertexTexture(glm::vec3 position, glm::vec2 st)
        : position(position), st(st) {}
};

struct VertexColorTexture {
    glm::vec3 position;
    glm::vec4 color;
    glm::vec2 st;

    VertexColorTexture() = default;
    VertexColorTexture(glm::vec3 position, glm::vec4 color, glm::vec2 st)
        : position(position), color(color), st(st) {}
};

Primitive::Primitive() {
    this->layout_color
        .begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float)
        .end();

    this->layout_texture
        .begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
        .end();

    this->layout_color_texture
        .begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float)
        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
        .end();
}

void Primitive::render_quad(
    glm::vec3 min, glm::vec3 max, glm::vec2 st_min, glm::vec2 st_max,
    const std::function<void(void)> &setup,
    const Program &program,
    bool invert_t, bgfx::ViewId view, u64 render_state) {
    const auto indices =
        std::array<u16, 6> { 0, 2, 1, 0, 3, 2 };

    auto vertices =
        util::make_array(
            VertexTexture(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)),
            VertexTexture(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f)),
            VertexTexture(glm::vec3(1.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f)),
            VertexTexture(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f)));

    if (invert_t) {
        for (usize i = 0; i < 4; i++) {
            vertices[i].st.t = 1.0f - vertices[i].st.t;
        }
    }

    bgfx::TransientVertexBuffer vertex_buffer;
    bgfx::TransientIndexBuffer index_buffer;

    bgfx::allocTransientIndexBuffer(&index_buffer, indices.size());
    std::memcpy(index_buffer.data, &indices[0], sizeof(indices));

    bgfx::allocTransientVertexBuffer(
        &vertex_buffer, vertices.size(), this->layout_texture);
    std::memcpy(vertex_buffer.data, &vertices[0], sizeof(vertices));

    // TODO: set render state if not already zero?
    auto model =
        glm::scale(
            glm::translate(glm::mat4(1.0f), min),
            max - min);

    bgfx::setTransform(reinterpret_cast<void *>(&model));
    bgfx::setVertexBuffer(0, &vertex_buffer);
    bgfx::setIndexBuffer(&index_buffer, 0, indices.size());
    bgfx::setState(render_state);
    setup();
    bgfx::submit(view, program.handle);
}

void Primitive::render_aabb(
    util::AABB aabb, glm::vec4 color, bgfx::ViewId view, u64 render_state) {
    const auto indices =
        std::array<u16, 36> {
            4, 7, 6, 4, 6, 5,   // (south (+z))
            3, 0, 1, 3, 1, 2,   // (north (-z))
            7, 3, 2, 7, 2, 6,   // (east  (+x))
            0, 4, 5, 0, 5, 1,   // (west  (-x))
            2, 1, 5, 2, 5, 6,   // (up    (+y))
            0, 3, 7, 0, 7, 4    // (down  (-y))
        };

    const auto vertices =
        util::make_array(
            VertexColor(glm::vec3(0, 0, 0), color),
            VertexColor(glm::vec3(0, 1, 0), color),
            VertexColor(glm::vec3(1, 1, 0), color),
            VertexColor(glm::vec3(1, 0, 0), color),
            VertexColor(glm::vec3(0, 0, 1), color),
            VertexColor(glm::vec3(0, 1, 1), color),
            VertexColor(glm::vec3(1, 1, 1), color),
            VertexColor(glm::vec3(1, 0, 1), color));

    bgfx::TransientVertexBuffer vertex_buffer;
    bgfx::TransientIndexBuffer index_buffer;

    bgfx::allocTransientIndexBuffer(&index_buffer, indices.size());
    std::memcpy(index_buffer.data, &indices[0], sizeof(indices));

    bgfx::allocTransientVertexBuffer(
        &vertex_buffer, vertices.size(), this->layout_color);
    std::memcpy(vertex_buffer.data, &vertices[0], sizeof(vertices));

    if (!render_state) {
        render_state =
            BGFX_STATE_WRITE_RGB
            | BGFX_STATE_WRITE_A
            | BGFX_STATE_DEPTH_TEST_LESS
            | BGFX_STATE_CULL_CW
            | BGFX_STATE_MSAA
            | BGFX_STATE_BLEND_ALPHA;
    }

    render_state |= 0; // triangle list

    auto model =
        glm::scale(
            glm::translate(glm::mat4(1.0f), aabb.min),
            aabb.size());
    bgfx::setTransform(reinterpret_cast<void *>(&model));
    bgfx::setVertexBuffer(0, &vertex_buffer);
    bgfx::setIndexBuffer(&index_buffer, 0, indices.size());
    bgfx::setState(render_state);

    auto program = state.renderer.programs["debug_color"].get();
    bgfx::submit(view, program->handle);
}
