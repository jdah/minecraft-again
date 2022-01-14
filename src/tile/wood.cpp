#include "util/util.hpp"
#include "level/area.hpp"
#include "tile/tile.hpp"
#include "state.hpp"

using namespace level;

struct TileWood : Tile {
    static constexpr TileId ID = 10;

    TileWood(TileId id) : Tile(id) {}

    glm::ivec2 texture_offset(
        Area &area,
        const glm::ivec3 pos,
        util::Direction dir) const override {
        return glm::vec2(6, 1);
    }
};

DECL_TILE_INITIALIZER(TileWood, wood, WOOD);
