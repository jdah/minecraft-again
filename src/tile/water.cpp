#include "util/util.hpp"
#include "level/area.hpp"
#include "tile/tile.hpp"
#include "state.hpp"

using namespace level;

struct TileWater : Tile {
    static constexpr TileId ID = 3;

    TileWater(TileId id) : Tile(id) {
        this->render_pass = RenderPass::WATER;
        this->material = Material::WATER;
    }

    glm::ivec2 texture_offset(
        Area &area,
        const glm::ivec3 pos,
        util::Direction dir) const override {
        return glm::ivec2(0, 15);
    }
};

DECL_TILE_INITIALIZER(TileWater, water, WATER);
