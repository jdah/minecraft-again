#ifndef GFX_FRAMEBUFFER_HPP
#define GFX_FRAMEBUFFER_HPP

#include "bgfx/bgfx.h"
#include "util/util.hpp"
#include "gfx/bgfx.hpp"

namespace gfx {
// thin auto-deleting wrapper around bgfx::FramebufferHandle
// non-copyable
struct Framebuffer {
    bgfx::FrameBufferHandle handle = { bgfx::kInvalidHandle };

    Framebuffer() = default;

    Framebuffer(bgfx::FrameBufferHandle handle) : handle(handle) {};

    Framebuffer(const Framebuffer &other) = delete;

    Framebuffer(Framebuffer &&other) : handle(other.handle) {
        other.handle = { bgfx::kInvalidHandle };
    }

    Framebuffer &operator=(const Framebuffer &other)= delete;

    Framebuffer &operator=(Framebuffer &&other) {
        this->handle = other.handle;
        other.handle = { bgfx::kInvalidHandle };
        return *this;
    }

    ~Framebuffer() {
        if (this->handle.idx != bgfx::kInvalidHandle) {
            util::log::out()
                << "Destroying framebuffer "
                << this->handle.idx
                << util::log::end;
            bgfx::destroy(this->handle);
        }
    }

    inline operator bgfx::FrameBufferHandle() {
        return this->handle;
    }
};

template <typename... Args>
auto make_attachment(Args... args) {
    bgfx::Attachment attachment;
    attachment.init(args...);
    return attachment;
}

}

#endif
