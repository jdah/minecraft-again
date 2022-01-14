#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "util/camera.hpp"
#include "util/time.hpp"
#include "util/types.hpp"
#include "util/util.hpp"
#include "level/area.hpp"

struct Player
    : util::Tickable, util::Updateable {
    static constexpr auto DRAG_COEFFICIENT = -0.02f;
    static constexpr auto HEIGHT = 1.65f;
    static constexpr auto GRAVITY =
        glm::vec3(0.0f, -0.52f / util::Time::TICKS_PER_SECOND, 0.0f);

    util::PerspectiveCamera camera;
    level::Area *area;

    bool flying = true, grounded = false, drag = true;
    glm::vec3 position, velocity;

    std::optional<std::tuple<glm::ivec3, util::Direction>> look;

    level::TileId place_tile;

    Player() = default;
    Player(const util::PerspectiveCamera &camera, level::Area *area);
    util::AABB aabb() const;
    void tick() override;
    void update() override;
    void render(bgfx::ViewId view = 0, u64 render_state = 0);
};

#endif
