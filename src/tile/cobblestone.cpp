#include "util/util.hpp"
#include "level/area.hpp"
#include "tile/tile.hpp"
#include "state.hpp"

using namespace level;

struct TileCobblestone : Tile {
    static constexpr TileId ID = 11;

    TileCobblestone(TileId id) : Tile(id) {}

    glm::ivec2 texture_offset(
        Area &area,
        const glm::ivec3 pos,
        util::Direction dir) const override {
        return glm::vec2(2, 2);
    }
};

DECL_TILE_INITIALIZER(TileCobblestone, cobblestone, COBBLESTONE);
