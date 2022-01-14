#ifndef GFX_PROGRAM_HPP
#define GFX_PROGRAM_HPP

#include "bgfx/bgfx.h"
#include "texture.hpp"
#include "util/util.hpp"
#include "gfx/util.hpp"

namespace gfx {
    // forward declaration
    struct Renderer;

struct Program {
    struct Uniform {
        std::string name;
        bgfx::UniformType::Enum type;
        u16 num;
        bgfx::UniformHandle handle = { bgfx::kInvalidHandle };
        bool destroy = false;

        Uniform() = default;
        Uniform(
            const std::string &name,
            bgfx::UniformType::Enum type,
            u16 num,
            bgfx::UniformHandle handle,
            bool destroy = true)
            : name(name),
              type(type),
              num(num),
              handle(handle),
              destroy(destroy) {};

        Uniform(const Uniform &other) = delete;

        Uniform(Uniform &&other) {
            *this = std::move(other);
        }

        Uniform &operator=(const Uniform &other) = delete;

        Uniform &operator=(Uniform &&other) {
            this->name = other.name;
            this->type = other.type;
            this->num = other.num;
            this->handle = other.handle;
            this->destroy = other.destroy;
            other.handle = { bgfx::kInvalidHandle };
            other.destroy = false;
            return *this;
        }

        ~Uniform() {
            if (this->destroy && bgfx::isValid(this->handle)) {
                bgfx::destroy(this->handle);
            }
        }

        inline operator bgfx::UniformHandle() {
            return this->handle;
        }
    };

    bgfx::ProgramHandle handle = { bgfx::kInvalidHandle };
    std::unordered_map<std::string, std::shared_ptr<Uniform>> uniforms;
    std::string name;

    Program(
        const std::string &name,
        Renderer &renderer,
        const std::string &vs_path,
        const std::string &fs_path);

    Program(const Program &other) = delete;

    Program(Program &&other) : handle(other.handle) {
        *this = std::move(other);
    }

    Program &operator=(const Program &other) = delete;

    Program &operator=(Program &&other) {
        this->handle = other.handle;
        other.handle = { bgfx::kInvalidHandle };
        this->uniforms = other.uniforms;
        this->name = other.name;
        return *this;
    }

    ~Program();

    inline operator bgfx::ProgramHandle() {
        return this->handle;
    }

    template <typename T>
    inline void set(
        const std::string &name, const T &value, u16 num = 1) {
        bgfx::setUniform(this->uniforms.at(name)->handle, &value, num);
    }

    template <typename T>
    inline bool try_set(
        const std::string &name, const T &value, u16 num = 1) {
        if (!this->uniforms.contains(name)) {
            return false;
        }

        bgfx::setUniform(this->uniforms.at(name)->handle, &value, num);
        return true;
    }

    // set override for textures
    inline void set(
        const std::string &name, u8 stage, const Texture &texture) {
        bgfx::setTexture(
            stage, this->uniforms.at(name)->handle, texture);
    }

    // try_set override for textures
    inline bool try_set(
        const std::string &name, u8 stage, const Texture &texture) {
        if (!this->uniforms.contains(name)) {
            return false;
        }

        this->set(name, stage, texture);
        return true;
    }
};
}

#endif
