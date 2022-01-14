#include "util/util.hpp"
#include "level/area.hpp"
#include "tile/tile.hpp"
#include "state.hpp"

using namespace level;

struct TileLeaves : Tile {
    static constexpr TileId ID = 7;

    TileLeaves(TileId id) : Tile(id) {
        this->transparency = Transparency::ON;
    }

    glm::ivec2 texture_offset(
        Area &area,
        const glm::ivec3 pos,
        util::Direction dir) const override {
        return glm::vec2(4, 1);
    }
};

DECL_TILE_INITIALIZER(TileLeaves, leaves, LEAVES);
