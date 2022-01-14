#include "level/chunk.hpp"
#include "level/area.hpp"
#include "state.hpp"

using namespace level;

/*  3D CUBE
 *  1-------2
 *  |5------+6
 *  ||      ||
 *  ||      ||
 *  0+------3|
 *   4-------7
 *
 *  bottom:
 *  4-------7
 *  |       |
 *  |       |
 *  |       |
 *  0-------3
 *
 * top:
 *  5-------6
 *  |       |
 *  |       |
 *  |       |
 *  1-------2
 *
 * vertices:
 *  (0, 0, 0)
 *  (0, 1, 0)
 *  (1, 1, 0)
 *  (1, 0, 0)
 *  (0, 0, 1)
 *  (0, 1, 1)
 *  (1, 1, 1)
 *  (1, 0, 1)
 *
 * indices:
 * 4, 7, 6, 4, 6, 5, // (south (+z))
 * 3, 0, 1, 3, 1, 2, // (north (-z))
 * 7, 3, 2, 7, 2, 6, // (east  (+x))
 * 0, 4, 5, 0, 5, 1, // (west  (-x))
 * 2, 1, 5, 2, 5, 6, // (up    (+y))
 * 0, 3, 7, 0, 7, 4  // (down  (-y))
 */

// indices, within each list of 6 cube indices, which represent are the 4
// unique vertices which make up each face
static const usize UNIQUE_INDICES[] = {0, 1, 2, 5};

// indices into emitted vertices which make up the two faces for a cube face
static const usize FACE_INDICES[] = {0, 1, 2, 0, 2, 3};

static const usize CUBE_INDICES[] = {
    4, 7, 6, 4, 6, 5, // (south (+z))
    3, 0, 1, 3, 1, 2, // (north (-z))
    7, 3, 2, 7, 2, 6, // (east  (+x))
    0, 4, 5, 0, 5, 1, // (west  (-x))
    2, 1, 5, 2, 5, 6, // (up    (+y))
    0, 3, 7, 0, 7, 4  // (down  (-y))
};

static const glm::vec3 CUBE_VERTICES[] = {
    glm::vec3(0, 0, 0),
    glm::vec3(0, 1, 0),
    glm::vec3(1, 1, 0),
    glm::vec3(1, 0, 0),
    glm::vec3(0, 0, 1),
    glm::vec3(0, 1, 1),
    glm::vec3(1, 1, 1),
    glm::vec3(1, 0, 1)
};

static const glm::vec3 CUBE_NORMALS[] = {
    glm::vec3( 0,  0,  1),
    glm::vec3( 0,  0, -1),
    glm::vec3( 1,  0,  0),
    glm::vec3(-1,  0,  0),
    glm::vec3( 0,  1,  0),
    glm::vec3( 0, -1,  0),
};

static const glm::vec2 CUBE_UVS[] = {
    glm::vec2(0, 0),
    glm::vec2(1, 0),
    glm::vec2(1, 1),
    glm::vec2(0, 1),
};

// static data for ChunkVertex
bgfx::VertexLayout ChunkRenderer::ChunkVertex::layout;

void ChunkRenderer::ChunkVertex::create_layout() {
    static bool initialized;

    if (initialized) {
        return;
    }

    layout
        .begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float, true)
        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float)
        .end();

    initialized = true;
}

ChunkRenderer::ChunkRenderer(Chunk &chunk)
    : chunk(chunk) {
    // TODO: pick a decent default size
    ChunkVertex::create_layout();
    this->vertex_buffer =
        util::RDUniqueResource<bgfx::DynamicVertexBufferHandle>(
            bgfx::createDynamicVertexBuffer(
                16,
                ChunkVertex::layout,
                BGFX_BUFFER_ALLOW_RESIZE),
            [](auto handle) { bgfx::destroy(handle); });
    this->index_buffer =
        util::RDUniqueResource<bgfx::DynamicIndexBufferHandle>(
            bgfx::createDynamicIndexBuffer(
                16,
                BGFX_BUFFER_ALLOW_RESIZE
                | BGFX_BUFFER_INDEX32),
            [](auto handle) { bgfx::destroy(handle); });
}

static void emit_face(
    std::vector<ChunkRenderer::ChunkVertex> &vertices,
    std::vector<u32> &indices,
    glm::vec3 position,
    glm::vec2 uv_offset,
    glm::vec2 uv_size,
    glm::vec4 material,
    util::Direction direction) {
    // index offset
    const usize offset = vertices.size();

    // emit vertices
    for (usize i = 0; i < 4; i++) {
        ChunkRenderer::ChunkVertex vertex;
        vertex.pos =
            position +
            CUBE_VERTICES[
                CUBE_INDICES[(direction * 6) + UNIQUE_INDICES[i]]];
        vertex.normal = CUBE_NORMALS[direction];
        vertex.uv = (CUBE_UVS[i] * uv_size) + uv_offset;
        vertex.material = material;
        vertices.push_back(vertex);
    }

    // emit indices
    for (usize i : FACE_INDICES) {
        indices.push_back(offset + i);
    }
}

static inline void emit_tile(
    ChunkRenderer &renderer,
    std::vector<ChunkRenderer::ChunkVertex> &vertices,
    std::vector<u32> &indices,
    glm::ivec3 pos) {

    auto &chunk = renderer.chunk;
    const auto pos_w = pos + chunk.offset_tiles;
    const auto &t = state.tiles[chunk.tiles[pos]];
    const auto uv_unit = glm::vec2(1.0f) / glm::vec2(16.0f);

    // pack material into vec4
    const auto &material = t.material;
    const glm::vec4 material_pack =
        glm::vec4(
            glm::vec3(0.0),
            static_cast<f32>(material.shininess) / 255.0f);

    for (auto d = util::Direction(0);
        d < util::Direction::COUNT;
        d++) {

        const auto n = pos + static_cast<glm::ivec3>(d);
        const auto &t_n = state.tiles[Chunk::TileData::from(chunk.or_area(n))];

        if (t_n.id == ID_AIR
            || (t_n.transparency != Tile::Transparency::OFF
                && (t_n.transparency != Tile::Transparency::MERGED
                    || t_n.id != t.id))) {
            const auto uv_offset = t.texture_offset(chunk.area, pos_w, d);

            emit_face(
                vertices, indices,
                glm::vec3(pos),
                glm::vec2(uv_offset.x, 16 - uv_offset.y - 1) * uv_unit,
                uv_unit,
                material_pack,
                d);
        }
    }
}

void ChunkRenderer::mesh() {
    // TODO: convert these to arena allocated (or preallocated) vectors
    struct Pass {
        std::vector<ChunkVertex> vertices;
        std::vector<u32> indices;

        Pass() : vertices(), indices() {};
    } passes[Tile::RenderPass::COUNT];

    glm::ivec3 pos;
    for (pos.x = 0; pos.x < Chunk::SIZE.x; pos.x++) {
        for (pos.y = 0; pos.y < Chunk::SIZE.y; pos.y++) {
            for (pos.z = 0; pos.z < Chunk::SIZE.z; pos.z++) {
                const TileId t = this->chunk[pos];
                if (t == 0) {
                    continue;
                }

                const auto pass = state.tiles[t].render_pass;
                emit_tile(
                    *this, passes[pass].vertices, passes[pass].indices, pos);
            }
        }
    }

    usize num_vertices = 0, num_indices = 0;

    // TODO: pre-size according to size of other vectors
    std::vector<ChunkVertex> merged_vertices;
    std::vector<u32> merged_indices;

    for (usize i = 0; i < Tile::RenderPass::COUNT; i++) {
        auto &vertices = passes[i].vertices;
        auto &indices = passes[i].indices;

        if (vertices.size() == 0 || indices.size() == 0) {
            this->pass_indices[i] = {
                .num_indices = 0,
                .indices_start = 0,
                .num_vertices = 0,
                .vertices_start = 0
            };
            continue;
        }

        this->pass_indices[i] = {
            .num_indices = indices.size(),
            .indices_start = num_indices,
            .num_vertices = vertices.size(),
            .vertices_start = num_vertices
        };

        merged_vertices.insert(
            merged_vertices.end(), vertices.begin(), vertices.end());

        merged_indices.insert(
            merged_indices.end(), indices.begin(), indices.end());

        num_vertices += vertices.size();
        num_indices += indices.size();
    }

    // must either be empty or have something
    util::_assert((num_indices == 0) == (num_vertices == 0));

    if (num_vertices > 0 && num_indices > 0) {
        bgfx::update(
            this->vertex_buffer,0,
            bgfx::copy(
                &merged_vertices[0],
                merged_vertices.size() * sizeof(merged_vertices[0])));

        bgfx::update(
            this->index_buffer, 0,
            bgfx::copy(
                &merged_indices[0],
                merged_indices.size() * sizeof(merged_indices[0])));
    }
}

void ChunkRenderer::render(
    Tile::RenderPass render_pass,
    bgfx::ViewId view, u64 render_state) {
    // re-mesh if dirty
    if (this->chunk.version != this->mesh_version &&
        state.throttles.mesh < state.throttles.mesh_max) {
        this->mesh();
        this->mesh_version = this->chunk.version;
        state.throttles.mesh++;
    }

    // empty mesh! gfx APIs will freak out if you submit with nothing
    if (this->mesh_version != this->chunk.version) {
        return;
    }

    if (!render_state) {
        render_state =
            BGFX_STATE_WRITE_MASK
            | BGFX_STATE_DEPTH_TEST_LESS
            | BGFX_STATE_CULL_CW;
    }

    render_state |= 0; // triangle list

    auto model =
        glm::translate(
            glm::mat4(1.0),
            glm::vec3(this->chunk.offset * Chunk::SIZE));
    bgfx::setTransform(reinterpret_cast<void *>(&model));

    if (this->pass_indices[render_pass].num_indices != 0) {
        bgfx::setVertexBuffer(
            0, this->vertex_buffer,
            this->pass_indices[render_pass].vertices_start,
            this->pass_indices[render_pass].num_vertices);
        bgfx::setIndexBuffer(
            this->index_buffer,
            this->pass_indices[render_pass].indices_start,
            this->pass_indices[render_pass].num_indices);
        bgfx::setState(render_state);

        gfx::Program *program = nullptr;
        switch (render_pass) {
            case Tile::DEFAULT:
                program = state.renderer.programs["chunk"].get();
                break;
            case Tile::WATER:
                program = state.renderer.programs["water"].get();
                program->try_set(
                    "time",
                    glm::vec4(state.time.ticks));
                program->try_set(
                    "s_noise", 1, *state.renderer.textures["noise"]);
                break;
            default:
                util::_assert(false);
        }

        program->try_set("s_tex", 0, *state.renderer.textures["blocks"]);
        bgfx::submit(view, *program);
    }
}
