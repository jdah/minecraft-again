#ifndef GFX_RENDERER_HPP
#define GFX_RENDERER_HPP

#include "bgfx/bgfx.h"
#include "util/util.hpp"
#include "gfx/util.hpp"
#include "gfx/program.hpp"
#include "gfx/primitive.hpp"
#include "gfx/texture.hpp"
#include "gfx/framebuffer.hpp"
#include "gfx/sun.hpp"

namespace gfx {
struct Renderer {
    std::unordered_map<
        u16,
        std::shared_ptr<Program::Uniform>> uniforms;
    std::unordered_map<
        std::string,
        std::unique_ptr<Program>> programs;
    std::unordered_map<
        std::string, std::unique_ptr<Texture>> textures;
    std::unordered_map<
        std::string, std::unique_ptr<Framebuffer>> framebuffers;
    std::unique_ptr<Primitive> primitive;

    bgfx::ViewId
        view_main = 0,
        view_gbuffer = 1,
        view_sun = 2,
        view_ssao = 3,
        view_ssao_blur = 4,
        view_light = 5,
        view_bloom_blur = 6,
        view_blur0 = 7,
        view_blur1 = 8,
        view_blur2 = 9;

    // bgfx capabilities
    bgfx::Caps capabilities;

    // size of window backbuffer
    glm::ivec2 target_size;

    // size of primary render target
    glm::ivec2 size;

    // primary look camera
    // set externally!
    util::Camera *look_camera;

    Sun sun;

    util::Moveable<bool> initialized;

    Renderer() = default;
    Renderer(const Renderer &other) = delete;
    Renderer(Renderer &&other) = default;
    Renderer &operator=(const Renderer &other) = delete;
    Renderer &operator=(Renderer &&other) = default;
    ~Renderer();

    void init();

    void prepare_frame();
    void end_frame();

    using RenderFn = std::function<void(bgfx::ViewId, u64)>;
    void composite(RenderFn render);
};

}

#endif
