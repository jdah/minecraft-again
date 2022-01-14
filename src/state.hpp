#ifndef STATE_HPP
#define STATE_HPP

#include "platform/platform.hpp"
#include "util/util.hpp"
#include "tile/tile.hpp"
#include "player.hpp"

struct State {
    platform::Platform platform;
    gfx::Renderer renderer;
    util::Time time;
    util::Bump frame_allocator;
    level::Tiles tiles;

    // TODO: remove this when proper entities are added
    Player player;

    struct {
        usize mesh, mesh_max = 8;
        usize gen, gen_max = 4;
    } throttles;
};

// global state, see main.cpp
extern State &state;

#endif
