#include "util/util.hpp"
#include "level/area.hpp"
#include "tile/tile.hpp"
#include "state.hpp"

using namespace level;

struct TileLog : Tile {
    static constexpr TileId ID = 6;

    TileLog(TileId id) : Tile(id) {}

    glm::ivec2 texture_offset(
        Area &area,
        const glm::ivec3 pos,
        util::Direction dir) const override {
        switch (dir) {
            case util::Direction::TOP:
            case util::Direction::BOTTOM:
                return glm::ivec2(3, 1);
            default:
                return glm::ivec2(2, 1);
        }
        return glm::vec2(0, 3);
    }
};

DECL_TILE_INITIALIZER(TileLog, log, LOG);
