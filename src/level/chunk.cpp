#include "level/chunk.hpp"
#include "level/area.hpp"

using namespace level;

Chunk::Data Chunk::or_area(const glm::ivec3 &pos) {
    if (Chunk::in_bounds(pos)) {
        return this->raw[pos];
    } else {
        return this->area.raw[this->offset_tiles + pos];
    }
}

Chunk *Chunk::neighbor(util::Direction d) {
    return this->area.chunkp(this->offset + static_cast<glm::ivec3>(d));
}

std::array<Chunk*, 6> Chunk::neighbors() {
    std::array<Chunk*, 6> res;

    for (const util::Direction d : util::Direction::ALL) {
        res[d] = this->neighbor(d);
    }

    return res;
}

void Chunk::tick() {

}
