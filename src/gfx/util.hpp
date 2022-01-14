#ifndef GFX_UTIL_HPP
#define GFX_UTIL_HPP

// bgfx
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>

#include "ext/stb_image.h"

#include "util/util.hpp"

namespace gfx {
inline std::string get_platform_shader(
    const std::string &path) {
    std::string platform;

    switch (bgfx::getRendererType()) {
        case bgfx::RendererType::Noop:
        case bgfx::RendererType::Direct3D9:  platform = "dx9";   break;
        case bgfx::RendererType::Direct3D11:
        case bgfx::RendererType::Direct3D12: platform = "dx11";  break;
        case bgfx::RendererType::Agc:
        case bgfx::RendererType::Gnm:        platform = "pssl";  break;
        case bgfx::RendererType::Metal:      platform = "metal"; break;
        case bgfx::RendererType::Nvn:        platform = "nvn";   break;
        case bgfx::RendererType::OpenGL:     platform = "glsl";  break;
        case bgfx::RendererType::OpenGLES:   platform = "essl";  break;
        case bgfx::RendererType::Vulkan:     platform = "spirv"; break;
        case bgfx::RendererType::WebGPU:     platform = "spirv"; break;
        default:
            // TODO
            break;
    }

    const auto [base, filename, ext] = util::split_path(path);
    return base + "/" + filename + "." + platform + ".bin";
}

inline util::Result<bgfx::ShaderHandle, std::string> load_shader(
    const std::string &path) {
    auto read = util::read_file(path);

    if (read.isErr()) {
        return util::Err(read.unwrapErr());
    }

    auto text = read.unwrap();
    auto res =
        bgfx::createShader(
            bgfx::copy(text.c_str(), text.length()));
    bgfx::setName(res, path.c_str());
    return util::Ok(res);
}

inline util::Result<std::tuple<glm::ivec2, bgfx::TextureHandle>, std::string>
   load_texture(const std::string &path) {
    // load image with stb_image
    glm::ivec2 size;
    int channels;
    stbi_set_flip_vertically_on_load(true);
    u8 *data = stbi_load(path.c_str(), &size.x, &size.y, &channels, 0);

    auto res =
        bgfx::createTexture2D(
            size.x, size.y,
            false, 1,
            bgfx::TextureFormat::RGBA8,
            BGFX_SAMPLER_U_CLAMP
            | BGFX_SAMPLER_V_CLAMP
            | BGFX_SAMPLER_MIN_POINT
            | BGFX_SAMPLER_MAG_POINT,
            bgfx::copy(data, size.x * size.y * channels));

    if (!bgfx::isValid(res)) {
        return util::Err("Error loading texture " + path);
    }

    std::free(data);
    return util::Ok(std::tuple(size, res));
}
}

#endif
