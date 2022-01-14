#ifndef TILE_TILE_HPP
#define TILE_TILE_HPP

#include "util/util.hpp"

namespace level {
    // forward declaration
    struct Area;

namespace Material {
    struct Material {
        u8 shininess;
    };

    static const inline Material
        WATER = {
            .shininess = 230,
        },
        METAL = {
            .shininess = 208,
        },
        DEFAULT = {
            .shininess = 0
        };
};

static constexpr usize MAX_TILES = 16384;

typedef u16 TileId;

struct Tile {
    enum RenderPass {
        DEFAULT = 0,
        WATER = 1,
        COUNT = (WATER + 1)
    };

    enum Transparency {
        OFF = 0,
        ON = 1,
        MERGED = 2
    };

    TileId id;
    RenderPass render_pass = DEFAULT;
    Material::Material material = Material::DEFAULT;
    Transparency transparency = Transparency::OFF;

    Tile() {};
    explicit Tile(TileId id);

    // implicit conversion to ID
    operator TileId() {
        return this->id;
    }

    virtual util::AABB aabb(
        Area &area, const glm::vec3 pos) const;

    virtual glm::ivec2 texture_offset(
        Area &area, const glm::ivec3 pos, util::Direction dir) const;
};

// global tile IDs
extern TileId
    ID_AIR,
    ID_GRASS,
    ID_DIRT,
    ID_WATER,
    ID_IRON,
    ID_STONE,
    ID_LOG,
    ID_LEAVES,
    ID_SAND,
    ID_GLASS,
    ID_WOOD,
    ID_COBBLESTONE;

// wrapper for global tile array
// TODO: this will start to cause problems if a subclass of Tile takes up more
// space than Tile itself! investigate solutions if subclasses need their own
// fields for some reason
struct Tiles final {
    Tiles();

    inline void init() {
#define _INIT_TILE(_name)       \
    extern void init_##_name(); \
    init_##_name();

        _INIT_TILE(air);
        _INIT_TILE(grass);
        _INIT_TILE(dirt);
        _INIT_TILE(water);
        _INIT_TILE(iron);
        _INIT_TILE(stone);
        _INIT_TILE(log);
        _INIT_TILE(leaves);
        _INIT_TILE(sand);
        _INIT_TILE(glass);
        _INIT_TILE(wood);
        _INIT_TILE(cobblestone);
    }

    inline Tile &operator[](usize i) {
        return this->raw[i];
    }

private:
    std::array<Tile, MAX_TILES> raw;
};

};

#define DECL_TILE_INITIALIZER(_class, _name, _caps)         \
    namespace level {                                       \
    TileId ID_##_caps;                                      \
    void init_##_name() {                                   \
        new (&state.tiles[_class::ID]) _class(_class::ID);  \
        ID_##_caps = state.tiles[_class::ID];               \
    }                                                       \
    }

#endif
