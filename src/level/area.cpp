#include "level/area.hpp"
#include "state.hpp"

using namespace level;

Area::Area(GeneratorFn generator) : generator(generator) {
    this->raw = AreaDataAccess<decltype(Chunk::raw)>(this, &Chunk::raw);
    this->tiles = AreaDataAccess<decltype(Chunk::tiles)>(this, &Chunk::tiles);
}

void Area::update() {

}

void Area::tick() {
    const auto
        center_offset = Area::to_offset(this->center),
        min_offset = center_offset - glm::ivec3(this->radius, 0, this->radius),
        max_offset = center_offset + glm::ivec3(this->radius, 0, this->radius);

    // remove chunks which are not in radius
    for (auto it = this->chunks.begin(); it != this->chunks.end();) {
        auto &[offset, chunk] = *it;

        if (offset.x < min_offset.x || offset.z < min_offset.z ||
            offset.x > max_offset.x || offset.z > max_offset.z) {
            this->chunks.erase(it++);
        } else {
            it++;
        }
    }

    // add chunks which should be in radius but are not loaded
    for (int x = min_offset.x; x <= max_offset.x; x++) {
        for (int z = min_offset.z; z <= max_offset.z; z++) {
            const auto offset = glm::ivec3(x, 0, z);

            if (!chunks.contains(offset) &&
                state.throttles.gen < state.throttles.gen_max) {
                auto chunk = new level::Chunk(*this, offset);
                this->chunks.emplace(offset, chunk);
                this->generator(*chunk);
                state.throttles.gen++;
            }
        }
    }


    for (auto &[_, chunk] : this->chunks) {
        chunk->tick();
    }
}

usize Area::get_colliders(
    const std::span<util::AABB> &dest, util::AABBi area) {
    usize n = 0;

    for (int x = area.min.x; x <= area.max.x; x++) {
        for (int y = area.min.y; y <= area.max.y; y++) {
            for (int z = area.min.z; z <= area.max.z; z++) {
                const auto pos = glm::ivec3(x, y, z);

                const TileId tile = this->tiles[pos];
                if (tile == 0) {
                    continue;
                }

                if (n >= dest.size()) {
                    util::log::out()
                        << util::log::ERROR
                        << "No more space in colliders container!"
                        << util::log::end;
                    goto end;
                }

                dest[n++] = state.tiles[tile].aabb(*this, pos);
            }

        }
    }

end:
    return n;
}
