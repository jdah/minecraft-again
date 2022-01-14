#include "tile/tile.hpp"
#include "state.hpp"

using namespace level;

Tiles::Tiles() {
    this->init();
}

Tile::Tile(TileId id) {
    if (state.tiles[id].id != 0) {
        util::log::out()
            << "Tile with ID " << id << " already exists"
            << util::log::ERROR << util::log::end;
        std::exit(1);
    }

    this->id = id;
}

util::AABB Tile::aabb(
    Area &area, const glm::vec3 pos) const {
    return util::AABB::unit().translate(pos);
}

glm::ivec2 Tile::texture_offset(
    Area &area, const glm::ivec3 pos, util::Direction dir) const {
    util::_assert(
        false,
        "Cannot get texture offset for tile " + std::to_string(this->id));
    return glm::ivec2(0);
}
