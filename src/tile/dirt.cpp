#include "util/util.hpp"
#include "level/area.hpp"
#include "tile/tile.hpp"
#include "state.hpp"

using namespace level;

struct TileDirt : Tile {
    static constexpr TileId ID = 2;

    TileDirt(TileId id) : Tile(id) {}

    glm::ivec2 texture_offset(
        UNUSED Area &area,
        UNUSED const glm::ivec3 pos,
        util::Direction dir) const override {
        return glm::ivec2(2, 0);
    }
};

DECL_TILE_INITIALIZER(TileDirt, dirt, DIRT);
