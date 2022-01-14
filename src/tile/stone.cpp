#include "util/util.hpp"
#include "level/area.hpp"
#include "tile/tile.hpp"
#include "state.hpp"

using namespace level;

struct TileStone : Tile {
    static constexpr TileId ID = 5;

    TileStone(TileId id) : Tile(id) {}

    glm::ivec2 texture_offset(
        Area &area,
        const glm::ivec3 pos,
        util::Direction dir) const override {
        return glm::ivec2(3, 0);
    }
};

DECL_TILE_INITIALIZER(TileStone, stone, STONE);
