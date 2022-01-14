#include "util/util.hpp"
#include "level/area.hpp"
#include "tile/tile.hpp"
#include "state.hpp"

using namespace level;

struct TileGrass : Tile {
    static constexpr TileId ID = 1;

    TileGrass(TileId id) : Tile(id) {}

    glm::ivec2 texture_offset(
        Area &area,
        const glm::ivec3 pos,
        util::Direction dir) const override {
        switch (dir) {
            case util::Direction::TOP:
                return glm::ivec2(0, 0);
            case util::Direction::BOTTOM:
                return glm::ivec2(2, 0);
            default:
                return glm::ivec2(1, 0);
        }
    }
};

DECL_TILE_INITIALIZER(TileGrass, grass, GRASS);
