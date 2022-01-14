#include "util/util.hpp"
#include "level/area.hpp"
#include "tile/tile.hpp"
#include "state.hpp"

using namespace level;

struct TileGlass : Tile {
    static constexpr TileId ID = 9;

    TileGlass(TileId id) : Tile(id) {
        this->transparency = Transparency::ON;
    }

    glm::ivec2 texture_offset(
        Area &area,
        const glm::ivec3 pos,
        util::Direction dir) const override {
        return glm::vec2(1, 1);
    }
};

DECL_TILE_INITIALIZER(TileGlass, glass, GLASS);
