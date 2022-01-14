#ifndef LEVEL_CHUNK_HPP
#define LEVEL_CHUNK_HPP

#include "util/util.hpp"
#include "gfx/gfx.hpp"

// TODO: figure out where to declare/define things for tiles
#include "tile/tile.hpp"

namespace level {

// forward declaration from area.hpp
struct Area;

struct Chunk final : util::Tickable {
    static constexpr const glm::ivec3 SIZE = glm::ivec3(16, 128, 16);
    static constexpr const usize VOLUME = SIZE.x * SIZE.y * SIZE.z;

    // chunk data type
    typedef u64 Data;

    // proxy for access to chunk data
    template <typename T, usize O, usize M, usize S>
    struct ChunkDataAccess final {
        // proxy for access to individual element
        struct Proxy {
            ChunkDataAccess *parent;
            Data *data;

            Proxy(ChunkDataAccess *parent, const glm::ivec3 &p)
                : Proxy(parent, p.x * SIZE.y * SIZE.z + p.y * SIZE.z + p.z) { }

            Proxy(ChunkDataAccess *parent, usize i)
                : parent(parent),
                  data(&(parent->chunk->data[i])) { }

            Proxy(ChunkDataAccess *parent, Data *data)
                : parent(parent), data(data) { }

            // TODO: this is much slower than it should be due to null check
            inline operator T() {
                return static_cast<T>(((*this->data) & M) >> O);
            }

            inline Proxy &operator=(T value) {
                // TODO: consider only doing this if value changes
                this->parent->chunk->version++;

                *this->data =
                    (*this->data & ~M)
                    | ((static_cast<Data>(value) << O) & M);
                return *this;
            }
        };

        // zeroable proxy, a little slower than a Proxy due to the null check
        // will also update neighboring chunks on assignment
        struct SafeProxy {
            Proxy p;
            glm::ivec3 pos;

            SafeProxy(ChunkDataAccess *parent, Data *data)
                : p(parent, data) { }
            SafeProxy(ChunkDataAccess *parent, const glm::ivec3 &pos)
                : p(parent, pos), pos(pos) { }

            inline operator T() {
                return this->p.data ? p : 0;
            }

            inline SafeProxy &operator=(T value) {
                // TODO: consider only doing this if value changes
                // update versions of neighboring chunks
                auto border = Chunk::border(this->pos);
                if (border) {
                    auto *neighbor = this->p.parent->chunk->neighbor(*border);
                    if (neighbor) {
                        neighbor->version++;
                    }
                }

                this->p = value;
                return *this;
            }
        };

        // proxy which evaluates to zero and crashes on assignment
        static const inline auto ZERO = SafeProxy(nullptr, nullptr);

        Chunk *chunk;

        ChunkDataAccess(Chunk *chunk) : chunk(chunk) { }

        inline Proxy operator[](usize index) {
            return Proxy(this, index);
        }

        inline auto operator[](const glm::ivec3 &p) {
            return Proxy(this, p);
        }

        inline auto safe(usize index) {
            return SafeProxy(this, index);
        }

        inline auto safe(const glm::ivec3 &p) {
            return SafeProxy(this, p);
        }

        static inline T from(Data d) {
            return static_cast<T>((d & M) >> O);
        }
    };

    std::array<Chunk::Data, Chunk::VOLUME> data;

    Area &area;
    glm::ivec3 offset, offset_tiles;

    // arbitrary integer, *must* change every time chunk data is updated
    // used for tracking when chunk is dirtied by external things (primarily
    // the renderer)
    u64 version;

    // data accessors
    // types are declared explicitly for easy use of their static methods
    using RawData =
        ChunkDataAccess<Data, 0, std::numeric_limits<Data>::max(), 64>;

    using TileData =
        ChunkDataAccess<TileId, 0, 0xFFFF, 16>;

    RawData raw;
    TileData tiles;

    Chunk(Area &area, glm::ivec3 offset)
        : area(area),
          offset(offset),
          offset_tiles(offset * SIZE),
          raw(this),
          tiles(this) {
        std::memset(&this->data, 0, sizeof(this->data));
    }
    Chunk(const Chunk &other) = default;
    Chunk(Chunk &&other) = default;

    inline auto operator[](usize index) {
        return this->raw[index];
    }

    inline auto operator[](glm::ivec3 p) {
        return this->raw[p];
    }

    // retrieve neighbor in specified direction
    // returns nullptr if not present
    Chunk *neighbor(util::Direction d);

    // retrieves data at the specified position or gets it from this chunk's
    // area if out of bounds
    Data or_area(const glm::ivec3 &pos);

    // retrieve all chunk neighbors
    // array entry is nullptr if not present
    std::array<Chunk*, 6> neighbors();

    void tick() override;

    // utility functions
    static inline bool in_bounds(const glm::ivec3 &pos) {
        return pos.x >= 0
            && pos.y >= 0
            && pos.z >= 0
            && pos.x < SIZE.x
            && pos.y < SIZE.y
            && pos.z < SIZE.z;
    }

    // returns true if the specified position is on a chunk border
    static inline bool on_border(const glm::ivec3 &pos) {
        return pos.x == 0
            || pos.y == 0
            || pos.z == 0
            || pos.x == SIZE.x - 1
            || pos.y == SIZE.y - 1
            || pos.z == SIZE.z - 1;
    }

    // gets the border of the specified postition, nullopt if not on border
    static inline std::optional<util::Direction> border(const glm::ivec3 &pos) {
        if (pos.x == 0) {
            return std::make_optional(util::Direction::WEST);
        } else if (pos.y == 0) {
            return std::make_optional(util::Direction::BOTTOM);
        } else if (pos.z == 0) {
            return std::make_optional(util::Direction::NORTH);
        } else if (pos.x == SIZE.x - 1) {
            return std::make_optional(util::Direction::EAST);
        } else if (pos.y == SIZE.y - 1) {
            return std::make_optional(util::Direction::TOP);
        } else if (pos.z == SIZE.z - 1) {
            return std::make_optional(util::Direction::SOUTH);
        }

        return std::nullopt;
    }
};

struct ChunkRenderer final {
    struct ChunkVertex {
        glm::vec3 pos;
        glm::vec3 normal;
        glm::vec2 uv;
        glm::vec4 material;

        ChunkVertex() = default;

        static void create_layout();

        // storage in chunk_renderer.cpp
        static bgfx::VertexLayout layout;
    };

    Chunk &chunk;

    // version of the chunk (Chunk::version) when it was last meshed
    usize mesh_version;

    util::RDUniqueResource<bgfx::DynamicIndexBufferHandle> index_buffer;
    util::RDUniqueResource<bgfx::DynamicVertexBufferHandle> vertex_buffer;

    // indices separate for default/water meshes
    struct {
        usize num_indices, indices_start;
        usize num_vertices, vertices_start;
    } pass_indices[Tile::RenderPass::COUNT];

    explicit ChunkRenderer(Chunk &chunk);
    ChunkRenderer(const ChunkRenderer &other) = delete;
    ChunkRenderer(ChunkRenderer &&other) = default;

    void mesh();
    void render(
        Tile::RenderPass render_pass,
        bgfx::ViewId view = 0, u64 render_state = 0);
};

}

#endif
