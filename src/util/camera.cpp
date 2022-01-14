#include "util/camera.hpp"
#include "util/log.hpp"
#include "util/std.hpp"

#include "gfx/program.hpp"

using namespace util;

void Camera::update(const glm::mat4 &view) {
    // empty, override-able in subclasses
}

void Camera::set_uniforms(const std::string &prefix, gfx::Program &p) {
    auto
        view_proj = this->proj * this->view,
        inv_proj = glm::inverse(this->proj),
        inv_view = glm::inverse(this->view),
        inv_view_proj = glm::inverse(view_proj);

    p.try_set(prefix + "_view", this->view, 1);
    p.try_set(prefix + "_proj", this->proj, 1);
    p.try_set(prefix + "_viewProj", view_proj, 1);
    p.try_set(prefix + "_invView", inv_view, 1);
    p.try_set(prefix + "_invProj", inv_proj, 1);
    p.try_set(prefix + "_invViewProj", inv_view_proj, 1);
}

OrthoCamera::OrthoCamera(
    glm::vec2 min, glm::vec2 max, glm::vec2 depth_range)
        : min(min), max(max), depth_range(depth_range) {
    this->update();
}

void OrthoCamera::update(const glm::mat4 &view) {
    this->proj =
        glm::ortho(
            this->min.x, this->max.x,
            this->min.y, this->max.y,
            this->depth_range.x, this->depth_range.y);

    if (view == glm::mat4(0)) {
        this->view =
            glm::translate(
                glm::mat4(1.0f),
                glm::vec3(this->position, 0.0f));
    } else {
        this->view = view;
    }
}

PerspectiveCamera::PerspectiveCamera(
    f32 fov, f32 aspect,
    glm::vec2 depth_range,
    glm::vec3 position)
        : position(position),
          fov(fov),
          aspect(aspect),
          depth_range(depth_range) {
    this->update();
}

void PerspectiveCamera::update(const glm::mat4 &view) {
    this->pitch =
        glm::clamp(
            this->pitch,
            -(glm::pi<f32>() / 2) + glm::epsilon<f32>(),
             (glm::pi<f32>() / 2) - glm::epsilon<f32>());
    this->yaw =
        (this->yaw < 0 ? (glm::pi<f32>() * 2) : 0.0f) +
            std::fmodf(this->yaw, glm::pi<f32>() * 2);
    this->direction =
        glm::normalize(
            glm::vec3(
                glm::cos(this->pitch) * glm::sin(this->yaw),
                glm::sin(this->pitch),
                glm::cos(this->pitch) * glm::cos(this->yaw)));
    this->right = glm::cross(this->direction, glm::vec3(0.0f, 1.0f, 0.0f));
    this->up = glm::cross(this->right, this->direction);

    if (view == glm::mat4(0)) {
        this->view =
            glm::lookAt(
                this->position,
                this->position + this->direction,
                this->up);
    } else {
        this->view = view;
    }

    this->proj =
        glm::perspective(
            this->fov, this->aspect,
            this->depth_range.x, this->depth_range.y);
}

void PerspectiveCamera::set_uniforms(
    const std::string &prefix, gfx::Program &p) {
    Camera::set_uniforms(prefix, p);
    p.try_set(prefix + "_position", glm::vec4(this->position, 1.0));
}

