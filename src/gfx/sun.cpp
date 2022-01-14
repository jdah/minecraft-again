#include "gfx/sun.hpp"

using namespace gfx;

void Sun::update(
    level::Area &area,
    const util::PerspectiveCamera &camera) {


    const auto old_view_proj = this->camera.proj * this->camera.view;

    // TODO: configured by area size
    const f32 distance = 128.0f;
    const f32 frustum_size = 256.0f;
    const glm::vec2 depth_range = glm::vec2(1.0f, 256.0f);

    const auto
        n_direction = -glm::normalize(this->direction),
        position = camera.position + (n_direction * distance);

    const auto
        min = glm::vec2(-frustum_size),
        max = glm::vec2(frustum_size);

    this->camera.proj =
        glm::ortho(
            min.x, max.x, min.y, max.y,
            depth_range.x, depth_range.y);
    this->camera.view =
        glm::lookAt(
            position,
            camera.position,
            glm::vec3(0.0f, 1.0f, 0.0f));

    // TODO: !!! BAD HACK
    static bool first = true;
    const auto view_proj = !first ? old_view_proj : (this->camera.proj * this->camera.view);
    first = false;

    // take real view center, project into shadow map space
    // additionally move from [-1, 1] NDC into [0, 1] space
    const auto center_s =
        (((view_proj * glm::vec4(camera.position, 1.0)).xyz()) * 0.5f) + 0.5f;

    // comput offset from nearest texel
    const auto texel = 1.0f / this->texture_size;
    const auto offset = glm::mod(center_s.xy(), texel);

    // translate center according to offset to get texel-quantized center
    // move back into [-1, 1] space
    const auto center_fixed =
        (glm::vec3(center_s.xy() - offset, center_s.z) * 2.0f) - 1.0f;

    // transform into world space
    const auto center_fixed_w =
        (glm::inverse(view_proj) * glm::vec4(center_fixed, 1.0)).xyz();

    // use corrected center in view matrix
    this->camera.view =
        glm::lookAt(
            position,
            center_fixed_w,
            glm::vec3(0.0f, 1.0f, 0.0f));
}

void Sun::set_uniforms(Program &program) {
    this->camera.set_uniforms("u_sun", program);
    program.try_set("u_sun_direction", glm::vec4(this->direction, 0.0));
    program.try_set("u_sun_diffuse", glm::vec4(this->diffuse, 1.0));
    program.try_set("u_sun_ambient", glm::vec4(this->ambient, 1.0));
}
