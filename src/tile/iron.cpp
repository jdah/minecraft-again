#include "util/util.hpp"
#include "level/area.hpp"
#include "tile/tile.hpp"
#include "state.hpp"

using namespace level;

struct TileIron : Tile {
    static constexpr TileId ID = 4;

    TileIron(TileId id) : Tile(id) {
        this->material = Material::METAL;
    }

    glm::ivec2 texture_offset(
        Area &area,
        const glm::ivec3 pos,
        util::Direction dir) const override {
        return glm::ivec2(0, 4);
    }
};

DECL_TILE_INITIALIZER(TileIron, iron, IRON);
