#include "level/gen.hpp"
#include "level/area.hpp"

using namespace level;

enum Biome {
    OCEAN,
    BEACH,
    PLAINS
};

constexpr int WATER_LEVEL = 64;

static inline void set(
    Chunk &chunk, const glm::ivec3 &pos, TileId tile,
    bool only_empty = false) {
    const auto
        pos_w = chunk.offset_tiles + pos,
        offset = Area::to_offset(pos_w);

    if (Chunk::in_bounds(pos) && (!only_empty || chunk.tiles[pos] == 0)) {
        chunk.tiles[pos] = tile;
    } else if (
        chunk.area.contains_chunk(offset)
        && (!only_empty || chunk.area.tiles[pos_w] == 0)) {
        chunk.area.tiles[pos_w] = tile;
    } else {
        chunk.area.out_of_bounds_tiles.push_back(
            std::make_tuple(pos_w, tile));
    }
}

static inline TileId get(Chunk &chunk, const glm::ivec3 &pos) {
    return chunk.tiles.safe(pos);
}

void tree(Chunk &chunk, util::Rand &rand, const glm::ivec3 &pos) {
    const auto under = get(chunk, pos - glm::ivec3(0, 1, 0));
    if (under != ID_GRASS && under != ID_DIRT) {
        return;
    }

    int h = rand.next<int>(4, 6);

    for (int y = pos.y; y <= pos.y + h; y++) {
        set(chunk, glm::ivec3(pos.x, y, pos.z), ID_LOG, true);
    }

    auto layer = [&](int s, int y_start, int height, f32 cc) {
        for (int xx = pos.x - s; xx <= pos.x + s; xx++) {
            for (int zz = pos.z - s; zz <= pos.z + s; zz++) {
                for (int yy = pos.y + y_start; yy < pos.y + y_start + height; yy++) {
                    bool corner =
                        (xx == pos.x - s || xx == pos.x + s)
                        && (zz == pos.z - s || zz == pos.z + s);

                    if (!corner ||
                        !(yy == (pos.y + y_start + height - 1)
                            && rand.next<f32>(0, 1) < cc)) {
                        set(chunk, glm::ivec3(xx, yy, zz), ID_LEAVES, true);
                    }
                }
            }
        }
    };

    int lh = rand.next<int>(2, 3), th = 2;
    layer(2, h - 1, lh, 0.4);
    layer(1, h - 1 + lh, th, 0.8);
}

void level::gen(Chunk &chunk) {
    const u64 seed = 4;
    auto rand = util::rand(1);

    // biome noise
    auto n = util::Octave(seed, 6, 0);

    auto os = util::make_array(
        util::Octave(seed, 8, 1),
        util::Octave(seed, 8, 2),
        util::Octave(seed, 8, 3),
        util::Octave(seed, 8, 4),
        util::Octave(seed, 8, 5),
        util::Octave(seed, 8, 6));

    auto cs = util::make_array(
        util::Combined(os[0], os[1]),
        util::Combined(os[2], os[3]),
        util::Combined(os[4], os[5]));

    for (int x = 0; x < Chunk::SIZE.x; x++) {
        for (int z = 0; z < Chunk::SIZE.z; z++) {
            const auto xz_w = glm::ivec2(x, z) + chunk.offset_tiles.xz();

            const f32 base_scale = 1.3f;
            int
                hr,
                hl = (cs[0].sample(glm::vec2(xz_w) * base_scale) / 6.0f) - 4.0f,
                hh = (cs[0].sample(glm::vec2(xz_w) * base_scale) / 6.0f) + 6.0f;

            // sample biome noise, extra noise
            f32 t = n.sample(xz_w),
                r = n.sample(-xz_w);
            hr = t > 0 ? hl : glm::max(hh, hl);

            // offset by water level to determine biome
            int h = hr + WATER_LEVEL;

            Biome biome;
            if (h < WATER_LEVEL) {
                biome = OCEAN;
            } else if (t < 0.08f && h < WATER_LEVEL + 2) {
                biome = BEACH;
            } else {
                biome = PLAINS;
            }

            // dirt/sand depth
            int d = r * 1.4f + 5.0f;

            TileId top;
            switch (biome) {
                case OCEAN:
                    if (r > 0.1f || t > 0.01f) {
                        top = ID_SAND;
                    } else {
                        top = ID_DIRT;
                    }
                    break;
                case BEACH:
                    top = ID_SAND;
                    break;
                default:
                    top = ID_GRASS;
                    break;
            }

            // build column
            for (int y = 0; y < h; y++) {
                TileId tile;

                if (y == (h - 1)) {
                    tile = top;
                } else if (y > (h - d)) {
                    if (top == ID_GRASS) {
                        tile = ID_DIRT;
                    } else {
                        tile = top;
                    }
                } else {
                    tile = ID_STONE;
                }

                chunk.tiles[glm::ivec3(x, y, z)] = tile;
            }

            for (int y = h; y < WATER_LEVEL; y++) {
                chunk.tiles[glm::ivec3(x, y, z)] = ID_WATER;
            }

            if (biome == PLAINS && rand.next<f32>(0, 1) < 0.001) {
                tree(chunk, rand, glm::ivec3(x, h, z));
            }
        }
    }

    // add out-of-bounds tiles in this chunk
    for (auto it = chunk.area.out_of_bounds_tiles.begin();
         it != chunk.area.out_of_bounds_tiles.end();) {
        auto [pos, tile] = *it;
        const auto pos_c = pos - chunk.offset_tiles;

        if (Chunk::in_bounds(pos_c)) {
            chunk.tiles[pos_c] = tile;
            it = chunk.area.out_of_bounds_tiles.erase(it);
        } else {
            it++;
        }
    }

    // bump versions of neighboring chunks
    for (auto *c : chunk.neighbors()) {
        if (c) {
            c->version++;
        }
    }
}
