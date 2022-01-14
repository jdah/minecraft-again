#include "util/util.hpp"
#include "level/area.hpp"
#include "tile/tile.hpp"
#include "state.hpp"

using namespace level;

struct TileSand : Tile {
    static constexpr TileId ID = 8;

    TileSand(TileId id) : Tile(id) {}

    glm::ivec2 texture_offset(
        Area &area,
        const glm::ivec3 pos,
        util::Direction dir) const override {
        return glm::vec2(0, 1);
    }
};

DECL_TILE_INITIALIZER(TileSand, sand, SAND);
