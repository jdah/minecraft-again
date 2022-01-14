#include "level/area.hpp"

using namespace level;

void AreaRenderer::render(
    Tile::RenderPass render_pass,
    bgfx::ViewId view, u64 render_state) {
    // ensure all chunks have renderers, get rid of those that are no longer
    // valid
    for (auto it = this->chunk_renderers.begin();
         it != this->chunk_renderers.end();) {
        auto &[offset, _] = *it;

        if (!this->area.contains_chunk(offset)) {
            this->chunk_renderers.erase(it++);
        } else {
            it++;
        }
    }

    for (auto &[offset, chunk] : this->area.chunks) {
        if (!this->chunk_renderers.contains(offset)) {
            this->chunk_renderers[offset] =
                std::make_unique<ChunkRenderer>(
                    level::ChunkRenderer(*chunk.get()));
        }
    }

    // TODO: frustum culling
    for (auto &[_, renderer] : this->chunk_renderers) {
        renderer->render(render_pass, view, render_state);
    }
}
