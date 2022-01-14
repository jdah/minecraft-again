#include "util/util.hpp"
#include "tile/tile.hpp"
#include "state.hpp"

using namespace level;

struct TileAir : Tile {
    static constexpr TileId ID = 0;

    TileAir(TileId id) : Tile(id) {
        this->transparency = Transparency::ON;
    }
};

DECL_TILE_INITIALIZER(TileAir, air, AIR);
