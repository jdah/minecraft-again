#include "bgfx/bgfx.h"
#include "platform/input.hpp"
#include "util/util.hpp"
#include "gfx/gfx.hpp"
#include "state.hpp"

// TODO: by #define/compile-time flag
#include "platform/glfw/platform_glfw.hpp"

#include "level/chunk.hpp"
#include "level/area.hpp"
#include "player.hpp"
#include <string>

// global state, referenced from state.hpp
static State global_state;
State &state = global_state;

std::unique_ptr<level::Area> area;
std::unique_ptr<level::AreaRenderer> area_renderer;

static void tick() {
    state.time.section_tick.begin();
    state.platform.tick();
    area->tick();
    state.player.tick();
    state.time.section_tick.end();
}

static void update() {
    state.time.section_update.begin();
    state.platform.update();
    state.player.update();
    state.time.update();

    state.time.section_update.end();

    for (usize i = 0; i < state.time.frame_ticks; i++) {
        tick();
    }
}

static void render() {
    // state.time.section_render.begin();

    // TODO: move debug text elsewhere
    std::vector<std::pair<std::string, f64>> times = {
        { "FRAME: ",    state.time.section_frame.avg()  },
        { "UPDATE: ",   state.time.section_update.avg() },
        { "RENDER: ",   state.time.section_render.avg() },
        { "TICK: ",     state.time.section_tick.avg()   },
    };

    int y = 0;
    for (const auto &[s, t] : times) {
        auto str =
            std::stringstream() << s
                << std::fixed << std::setprecision(3)
                << util::Time::to_millis(t) << " ms";
        // bgfx::dbgTextPrintf(0, y, ((0x2 + y) << 4) | 0xF, str.str().c_str());
        y++;
    }

    // state.time.section_render.end();
}

int main(UNUSED int argc, UNUSED char *argv[]) {
    state.frame_allocator = util::Bump(16384);
    state.time = util::Time([](){
            return
                std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::high_resolution_clock::now()
                        .time_since_epoch()).count();
        });
    state.platform.resources_path = "res";
    state.platform.log_out = &std::cout;
    state.platform.log_err = &std::cerr;

    state.platform.settings =
        toml::parse(
            util::read_file(state.platform.resources_path + "/defaults.toml")
                .unwrap());

    state.platform.window =
        std::unique_ptr<platform::GLFW::Window>(
            new platform::GLFW::Window(
                glm::ivec2(
                    state.platform.settings["gfx"]["width"].value_or(1280),
                    state.platform.settings["gfx"]["height"].value_or(720)),
                "GAME"));

    state.renderer.init();
    state.platform.inputs["mouse"] =
        std::unique_ptr<platform::Input>(
            new platform::GLFW::Mouse(
                *dynamic_cast<platform::GLFW::Window*>(
                    state.platform.window.get())));
    state.platform.inputs["keyboard"] =
        std::unique_ptr<platform::Input>(
            new platform::GLFW::Keyboard(
                *dynamic_cast<platform::GLFW::Window*>(
                    state.platform.window.get())));

    // TODO: do elsewhere
    // capture mouse
    state.platform.get_input<platform::Mouse>()
        .set_mode(platform::Mouse::DISABLED);

    area = std::make_unique<level::Area>(level::gen);
    area_renderer = std::make_unique<level::AreaRenderer>(*area);

    auto window_size = state.platform.window->get_size();
    state.player =
        Player(
            util::PerspectiveCamera(
                glm::radians(75.0f),
                window_size.x / (f32) window_size.y,
                glm::vec2(0.08f, 128.0f)),
            area.get());

    float time = 0.0;
    while (!state.platform.window->is_close_requested()) {
        state.time.section_frame.begin();
        bgfx::dbgTextClear();

        time += 0.3f;
        state.platform.window->prepare_frame();
        state.renderer.prepare_frame();

        // TODO: remove this
        area->center = glm::ivec3(state.player.position);

        // TODO: remove this too
        auto &keyboard = state.platform.get_input<platform::Keyboard>();

        for (usize i = 0; i < 10; i++) {
            const auto key = keyboard[std::to_string(i)];
            if (key && (*key)->pressed) {
                state.player.place_tile = static_cast<level::TileId>(i + 1);
            }
        }

        if (keyboard["q"] && (*keyboard["q"])->pressed) {
            state.player.place_tile = level::ID_WOOD;
        }

        if (keyboard["p"] && (*keyboard["p"])->pressed) {
            state.player.flying = !state.player.flying;
        }

        auto &composite = *state.renderer.programs["composite"];
        composite.try_set("u_show_buffer", glm::vec4(0, 0, 0, 0));

        update();

        state.time.section_render.begin();
        render();

        state.renderer.look_camera = &state.player.camera;
        // TODO: do this in the renderer!!!
        state.renderer.sun.direction = glm::vec3(0.60f, -0.7f, -0.30f);
        state.renderer.sun.update(*area, state.player.camera);
        // TODO: !!!
        state.renderer.composite(
            [&](bgfx::ViewId view, u64 flags) {
                area_renderer->render(
                    level::Tile::RenderPass::DEFAULT, view, flags);
                area_renderer->render(
                    level::Tile::RenderPass::WATER, view, flags);
            });

        state.throttles.gen = 0;
        state.throttles.mesh = 0;

        state.renderer.end_frame();
        state.platform.window->end_frame();
        state.time.section_render.end();
        state.time.section_frame.end();
        state.frame_allocator.clear();
    }

    // TODO: remove
    area_renderer.reset();
    area.reset();

    util::log::print("Exiting normally");
    return 0;
}
