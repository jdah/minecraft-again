#include "player.hpp"
#include "glm/ext/scalar_constants.hpp"
#include "state.hpp"

Player::Player(const util::PerspectiveCamera &camera, level::Area *area)
    : camera(camera), area(area) {
    this->position = glm::vec3(2, 80, 2);
    this->velocity = glm::vec3(0.0f);
}

util::AABB Player::aabb() const {
    return
        util::AABB::unit()
            .scale(glm::vec3(0.25f, 1.8f, 0.25f))
            .translate(glm::vec3(0.0f, this->position.y, 0.0f))
            .center_on(this->position, glm::bvec3(1, 0, 1));
}

// returns legal movement for the AABB on the specified axis
static f32 move_axis(
    util::AABB box,
    f32 movement,
    std::span<util::AABB> colliders,
    glm::vec3 axis) {
    static constexpr auto EPSILON = 0.05f;

    auto d_v = axis * movement;
    auto sign = glm::sign(movement);
    auto index = axis.x != 0 ? 0 : (axis.y != 0 ? 1 : 2);

    util::AABB moved = box.translate(d_v);

    for (const auto &c : colliders) {
        if (!c.collides(moved)) {
            continue;
        }

        // compute collision depth, resolve, and re-translate
        auto depth = moved.depth(c)[index];
        d_v[index] += -sign * (depth + EPSILON);
        moved = box.translate(d_v);

        // zero movement if it was effectively stopped by this collision
        if (glm::abs(d_v[index]) <= EPSILON) {
            d_v[index] = 0.0f;
            break;
        }
    }

    auto result = d_v[index];
    return glm::abs(result) <= glm::epsilon<f32>() ? 0.0f : result;
}

static glm::vec3 move(
    util::AABB box,
    glm::vec3 movement,
    std::span<util::AABB> colliders) {
    glm::vec3 result;
    util::AABB current = box;

    #pragma unroll
    for (usize i = 0; i < 3; i++) {
        glm::vec3 axis(0);
        axis[i] = 1.0f;

        f32 movement_axis = move_axis(box, movement[i], colliders, axis);
        current = current.translate(axis * movement_axis);
        result[i] = movement_axis;
    }

    return result;
}

void Player::tick() {
    auto &keyboard = state.platform.get_input<platform::Keyboard>();
    auto &mouse = state.platform.get_input<platform::Mouse>();

    auto
        btn_forward = keyboard["w"],
        btn_backward = keyboard["s"],
        btn_left = keyboard["a"],
        btn_right = keyboard["d"],
        btn_up = keyboard["space"],
        btn_down = keyboard["left_shift"],
        btn_jump = keyboard["space"],
        btn_place = mouse["right"],
        btn_destroy = mouse["left"];

    auto
        direction = glm::vec3(0),
        forward =
            glm::vec3(
                glm::sin(this->camera.yaw),
                0.0f,
                glm::cos(this->camera.yaw)),
        right = glm::cross(forward, glm::vec3(0, 1, 0));

    if (btn_forward && (*btn_forward)->down) {
        direction += forward;
    }

    if (btn_backward && (*btn_backward)->down) {
        direction -= forward;
    }

    if (btn_left && (*btn_left)->down) {
        direction -= right;
    }

    if (btn_right && (*btn_right)->down) {
        direction += right;
    }

    if (this->flying && btn_up && (*btn_up)->down) {
        direction += glm::vec3(0, 1, 0);
    }

    if (this->flying && btn_down && (*btn_down)->down) {
        direction -= glm::vec3(0, 1, 0);
    }

    if (!this->flying && this->grounded &&
        btn_jump && (*btn_jump)->down) {
        // jump
        this->velocity += glm::vec3(0.0f, 0.20f, 0.0f);
    }

    direction = glm::normalize(direction);

    glm::vec3 movement(0.0f);
    if (!glm::any(glm::isnan(direction))) {
        const auto speed = 2.4f / util::Time::TICKS_PER_SECOND;
        movement = speed * direction;
    }

    f32 xz_coefficient;
    if (this->flying) {
        xz_coefficient = 0.4f;
    } else if (!this->grounded) {
        xz_coefficient = 0.04f;
    } else {
        xz_coefficient = 1.0f;
    }

    movement *= xz_coefficient;
    this->velocity += movement;

    // gravity if not flying
    if (!this->flying) {
        this->velocity += Player::GRAVITY;
    }

    // extra walking drag on ground
    if (this->grounded) {
        this->velocity *= glm::vec3(0.6f, 1.0f, 0.6f);
    }

    // simulate drag (in proportion to velocity!)
    this->velocity += Player::DRAG_COEFFICIENT * this->velocity;

    std::array<util::AABB, 256> colliders;

    usize n =
        area->get_colliders(
            colliders,
            util::AABBi::unit()
                .scale(4)
                .center_on(glm::floor(this->camera.position)));

    // attempt movement by velocity
    auto moved = move(this->aabb(), this->velocity, { &colliders[0], n });

    // zero velocity on axes which did not completely move (stopped)
    glm::bvec3 stopped(false);
    for (usize i = 0; i < 3; i++) {
        if (glm::abs(this->velocity[i] - moved[i]) >= glm::epsilon<f32>()) {
            stopped[i] = true;
            this->velocity[i] = 0.0f;
        }
    }

    // if y velocity was stopped, player is on the ground
    this->grounded = this->velocity.y <= glm::epsilon<f32>() && stopped.y;

    this->position += this->velocity;
    this->camera.position =
        this->position + glm::vec3(0.0f, Player::HEIGHT, 0.0f);

    this->look =
        util::Ray(this->camera.position, this->camera.direction)
            .intersect_block(
                [&](auto p){ return this->area->tiles[p] != 0; },
                4.0f);

    if (this->look) {
        const auto to_place = state.tiles[this->place_tile];

        const auto &[look_pos, look_face] = *this->look;
        const auto place_pos = look_pos + static_cast<glm::ivec3>(look_face);

        if (btn_place && (*btn_place)->pressed_tick) {
            const auto aabb =
                to_place.aabb(*this->area, place_pos);

            // only allow placement if it's not on top of player
            if (!this->aabb().collides(aabb)) {
                this->area->tiles[place_pos] = to_place.id;
            }
        }

        if (btn_destroy && (*btn_destroy)->pressed_tick) {
            this->area->tiles[look_pos] = level::ID_AIR;
        }
    }
}

void Player::update() {
    auto sensitivity =
        state.platform.settings["mouse"]["sensitivity"].value_or(1.0f);
    auto &mouse = state.platform.get_input<platform::Mouse>();
    this->camera.yaw -= sensitivity * 2.5f * mouse.pos_delta_n.x;
    this->camera.pitch += sensitivity * 2.5f * mouse.pos_delta_n.y;

    this->camera.update();
}

void Player::render(bgfx::ViewId view, u64 render_state) {
    if (this->look) {
        const auto &[look_block, look_face] = *this->look;
        const auto look_tile = state.tiles[this->area->tiles[look_block]];

        state.renderer.primitive->render_aabb(
            look_tile.aabb(*this->area, look_block).scale_center(1.005f),
            glm::vec4(
                glm::vec3(1.0),
                0.6f * glm::abs(
                    glm::cos(
                        ((state.time.ticks % 120) / 120.0f)
                            * (glm::pi<f32>() * 2)))),
            view, render_state);
    }
}
