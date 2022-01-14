#ifndef GFX_TEXTURE_HPP
#define GFX_TEXTURE_HPP

#include "util/util.hpp"
#include "gfx/bgfx.hpp"

namespace gfx {
// thin auto-deleting wrapper around bgfx::TextureHandle
// non-copyable
struct Texture {
    bgfx::TextureHandle handle = { bgfx::kInvalidHandle };
    std::optional<glm::ivec2> size = std::nullopt;

    Texture() = default;

    Texture(bgfx::TextureHandle handle) : handle(handle) {};

    Texture(
        bgfx::TextureHandle handle,
        glm::ivec2 size)
        : handle(handle), size(size) {};

    Texture(std::tuple<glm::ivec2, bgfx::TextureHandle> t)
        : handle(std::get<1>(t)), size(std::get<0>(t)) {};

    Texture(const Texture &other) = delete;

    Texture(Texture &&other) : handle(other.handle) {
        other.handle = { bgfx::kInvalidHandle };
    }

    Texture &operator=(const Texture &other) = delete;

    Texture &operator=(Texture &&other) {
        this->handle = other.handle;
        other.handle = { bgfx::kInvalidHandle };
        return *this;
    }

    ~Texture() {
        if (this->handle.idx != bgfx::kInvalidHandle) {
            util::log::out()
                << "Destroying texture "
                << this->handle.idx
                << util::log::end;
            bgfx::destroy(handle);
        }
    }

    inline operator bgfx::TextureHandle() const {
        return this->handle;
    }
};
}

#endif
