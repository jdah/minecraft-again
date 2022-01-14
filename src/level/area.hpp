#ifndef LEVEL_AREA_HPP
#define LEVEL_AREA_HPP

#include "util/util.hpp"
#include "level/chunk.hpp"
#include "level/gen.hpp"

namespace level {
struct Area final
    : util::Updateable, util::Tickable {

    // proxy for access to area data through chunk data proxy
    template <typename T>
    struct AreaDataAccess final {
        Area *area;
        T Chunk::*member;

        AreaDataAccess() = default;
        AreaDataAccess(
            Area *area,
            T Chunk::*member)
            : area(area), member(member) { };

        // returns a SafeProxy, use direct chunk access for better speed
        inline auto operator[](const glm::ivec3 &pos) {
            const auto offset = Area::to_offset(pos);
            return
                area->contains_chunk(offset) ?
                    (this->area->chunk(offset).*this->member)
                        .safe(Area::to_chunk_pos(pos))
                    : T::ZERO;
        }
    };

    // data access proxies
    AreaDataAccess<decltype(Chunk::raw)> raw;
    AreaDataAccess<decltype(Chunk::tiles)> tiles;

    // chunk data
    std::unordered_map<glm::ivec3, std::unique_ptr<Chunk>> chunks;

    // data which was set outside of world bounds
    std::vector<std::tuple<glm::ivec3, TileId>> out_of_bounds_tiles;

    // TODO: replace when entities are added
    glm::ivec3 center;

    using GeneratorFn = std::function<void(Chunk &)>;
    GeneratorFn generator;
    usize radius = 10;

    explicit Area(GeneratorFn generator);

    void update() override;
    void tick() override;

    usize get_colliders(
        const std::span<util::AABB> &dest, util::AABBi area);

    // returns true if the area contains the chunk at the specified offset
    // AND it is loaded in (present in the area)
    inline bool contains_chunk(const glm::ivec3 &offset) {
        return this->chunks.contains(offset);
    }

    // get a raw chunk pointer
    inline Chunk *chunkp(const glm::ivec3 &offset) {
        return
            this->chunks.contains(offset) ?
                this->chunks[offset].get() :
                nullptr;
    }

    // get a raw chunk reference (crashes if chunk is not present!)
    inline Chunk &chunk(const glm::ivec3 &offset) {
        return *this->chunks[offset].get();
    }

    // raw chunk data access via area offset
    inline auto operator[](const glm::ivec3 &pos) {
        auto *chunk = this->chunks[Area::to_offset(pos)].get();
        return chunk ? (*chunk)[Area::to_chunk_pos(pos)] : 0;
    }

    // utility functions
    // area pos to chunk pos
    static inline glm::ivec3 to_chunk_pos(glm::ivec3 pos_a) {
        // (((pos % size) + size) % size) to deal with negatives correctly
        return ((pos_a % Chunk::SIZE) + Chunk::SIZE) % Chunk::SIZE;
    }

    // area pos to chunk offset
    static inline glm::ivec3 to_offset(const glm::ivec3 &pos_a) {
        return glm::floor(glm::vec3(pos_a) / glm::vec3(Chunk::SIZE));
    }

    // float to tile pos
    static inline glm::ivec3 to_tile(const glm::vec3 &pos_f) {
        return glm::floor(pos_f);
    }
};

struct AreaRenderer final {
    Area &area;
    std::unordered_map<glm::ivec3, std::unique_ptr<ChunkRenderer>>
        chunk_renderers;

    explicit AreaRenderer(Area &area)
        : area(area) { }

    void render(
        Tile::RenderPass render_pass,
        bgfx::ViewId view = 0, u64 render_state = 0);
};
}

#endif
