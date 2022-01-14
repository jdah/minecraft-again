#include "gfx/program.hpp"
#include "gfx/renderer.hpp"
#include "state.hpp"

using namespace gfx;

Program::Program(
    const std::string &name,
    Renderer &renderer,
    const std::string &vs_path,
    const std::string &fs_path) {
    this->name = name;

    util::log::out()
        << "Loading program"
        << " (" << this->name << ") "
        << vs_path
        << " + " << fs_path
        << util::log::end;

    auto
        vs = load_shader(get_platform_shader(vs_path)).unwrap(),
        fs = load_shader(get_platform_shader(fs_path)).unwrap();

    this->handle = bgfx::createProgram(vs, fs, true);
    util::_assert(bgfx::isValid(this->handle), "Error loading shader");

    util::log::out() << "  handle: " << this->handle.idx << util::log::end;

    const auto
        n_vs = bgfx::getShaderUniforms(vs),
        n_fs = bgfx::getShaderUniforms(fs);

    std::vector<bgfx::UniformHandle> handles(n_vs + n_fs);
    if (n_vs > 0) {
        bgfx::getShaderUniforms(vs, &handles[0], n_vs);
    }

    if (n_fs > 0) {
        bgfx::getShaderUniforms(fs, &handles[n_vs], n_fs);
    }

    for (const auto handle : handles) {
        bgfx::UniformInfo info;
        bgfx::getUniformInfo(handle, info);

        // check if uniform already exists in renderer
        if (renderer.uniforms.contains(handle.idx)) {
            this->uniforms[info.name] = renderer.uniforms[handle.idx];
            util::log::out() << "  (already exists)" << util::log::end;
        } else {
            auto uniform =
                std::make_shared<Uniform>(
                    info.name, info.type, info.num, handle, false);
            renderer.uniforms[handle.idx] = uniform;
            this->uniforms[info.name] = uniform;
        }

        util::log::out() << "  uniform: " << info.name << util::log::end;
    }
}

Program::~Program() {
    util::log::out()
        << "Destroying program " << this->name
        << " (" << this->handle.idx << ")"
        << util::log::end;

    bgfx::destroy(this->handle);
}
