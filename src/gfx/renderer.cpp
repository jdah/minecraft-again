#include "gfx/renderer.hpp"
#include "bgfx/bgfx.h"
#include "state.hpp"
#include "util/camera.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "ext/stb_image.h"

// TODO: remove
#include "gfx/sky.hpp"

using namespace gfx;

static u64 get_reset_flags(Renderer &renderer) {
    return
        state.platform.settings["gfx"]["vsync"].value_or(true) ?
            BGFX_RESET_VSYNC : BGFX_RESET_NONE;
}

static constexpr u64 default_buffer_flags =
    BGFX_SAMPLER_MIN_POINT
    | BGFX_SAMPLER_MAG_POINT
    | BGFX_SAMPLER_MIP_POINT
    | BGFX_SAMPLER_U_CLAMP
    | BGFX_SAMPLER_V_CLAMP;

static auto make_buffer(
    glm::vec2 size,
    bgfx::TextureFormat::Enum fmt = bgfx::TextureFormat::Count,
    usize flags = 0) {
    fmt = fmt != bgfx::TextureFormat::Count ? fmt : bgfx::TextureFormat::BGRA8;
    flags = flags ? flags : BGFX_TEXTURE_RT | default_buffer_flags;
    return std::make_unique<Texture>(
        bgfx::createTexture2D(size.x, size.y, false, 1, fmt, flags),
        size);
}

static auto make_framebuffer(
    bgfx::ViewId view,
    const std::vector<bgfx::Attachment> &attachments) {
    auto framebuffer =
        std::make_unique<Framebuffer>(
            bgfx::createFrameBuffer(
                attachments.size(),
                &attachments[0]));
    bgfx::setViewFrameBuffer(view, framebuffer->handle);
    return framebuffer;
};

static auto make_view(
    bgfx::ViewId view,
    glm::vec2 size,
    u64 clear = BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH,
    u64 clear_color = 0) {
    bgfx::setViewClear(view, clear, clear_color, 1.0f, 0);
    bgfx::setViewRect(view, 0, 0, size.x, size.y);

    // TODO: use a different view mode?
    // bgfx::setViewMode(view, bgfx::ViewMode::Sequential);
}

void Renderer::init() {
    this->target_size = state.platform.window->get_size();
    this->size = this->target_size;

    // initialize bgfx
    bgfx::Init init;

    bgfx::PlatformData platform_data;
    state.platform.window->set_platform_data(platform_data);

    init.platformData = platform_data;

    // TODO: let system choose renderer
    init.type = bgfx::RendererType::Count;
    init.resolution.width = this->target_size.x;
    init.resolution.height = this->target_size.y;
    init.resolution.reset = get_reset_flags(*this);

    if (!bgfx::init(init)) {
        util::log::print(
            "Error initializing BGFX",
            util::log::Level::ERROR);
        std::exit(1);
    }

    // mark initialized so renderer is properly destructed
    this->initialized = true;

    bgfx::reset(
        this->target_size.x, this->target_size.y,
        get_reset_flags(*this),
        init.resolution.format);
    bgfx::setDebug(BGFX_DEBUG_TEXT);

    this->capabilities = *bgfx::getCaps();
    this->primitive = std::unique_ptr<Primitive>(new Primitive());

    // generate noise texture
    const auto noise_size = glm::vec2(128, 128);
    this->textures["noise"] =
        std::make_unique<Texture>(
            bgfx::createTexture2D(
                noise_size.x, noise_size.y, false, 1,
                bgfx::TextureFormat::BGRA8,
                BGFX_TEXTURE_RT
                | BGFX_SAMPLER_MIN_POINT
                | BGFX_SAMPLER_MAG_POINT
                | BGFX_SAMPLER_U_MIRROR
                | BGFX_SAMPLER_V_MIRROR),
            noise_size);

    auto rand = util::rand(0x0153);
    auto noise_data = std::vector<u8>(noise_size.x * noise_size.y * 4);
    for (usize i = 0; i < noise_data.size(); i++) {
        noise_data[i] = rand.next<u8>(0, 255);
    }
    bgfx::updateTexture2D(
        *this->textures["noise"], 0, 0,
        0, 0, noise_size.x, noise_size.y,
        bgfx::copy(&noise_data[0], noise_data.size() * sizeof(u8)));

    this->textures["gbuffer"] =
        make_buffer(this->target_size, bgfx::TextureFormat::BGRA8);

    this->textures["normal"] =
        make_buffer(this->target_size, bgfx::TextureFormat::BGRA8);

    this->textures["depth"] =
        make_buffer(this->target_size, bgfx::TextureFormat::D32F);

    this->textures["light"] =
        make_buffer(this->target_size, bgfx::TextureFormat::BGRA8);

    this->textures["bloom"] =
        make_buffer(this->target_size, bgfx::TextureFormat::BGRA8);

    this->textures["bloom_blur"] =
        make_buffer(this->target_size, bgfx::TextureFormat::BGRA8);

    this->textures["ssao"] =
        make_buffer(this->target_size, bgfx::TextureFormat::BGRA8);

    this->textures["ssao_blur"] =
        make_buffer(this->target_size, bgfx::TextureFormat::BGRA8);

    this->textures["blur0"] =
        make_buffer(this->target_size, bgfx::TextureFormat::BGRA8);

    this->textures["blur1"] =
        make_buffer(this->target_size, bgfx::TextureFormat::BGRA8);

    this->textures["blur2"] =
        make_buffer(this->target_size, bgfx::TextureFormat::BGRA8);

    this->sun = Sun(4096);
    this->textures["sun_depth"] =
        make_buffer(
            glm::vec2(this->sun.texture_size),
            bgfx::TextureFormat::D32F, 0);

    // load all programs in shaders directory
    for (const auto &p :
            util::list_files(state.platform.resources_path + "/shaders")
                .unwrap()) {
        if (util::is_directory(p).unwrap()) {
            const auto filename = std::get<1>(util::split_path(p));
            this->programs[filename] =
                std::make_unique<Program>(
                    filename,
                    *this,
                    p + "/vs_" + filename + ".sc",
                    p + "/fs_" + filename + ".sc");
        }
    }

    // TODO: automatic?
    this->textures["blocks"] =
        std::make_unique<Texture>(load_texture("res/blocks.png").unwrap());

    // configure views
    make_view(this->view_main, this->target_size);

    // GBUFFER
    make_view(this->view_gbuffer, this->target_size);
    this->framebuffers["gbuffer"] =
        make_framebuffer(
            this->view_gbuffer,
            {
                make_attachment(this->textures["gbuffer"]->handle),
                make_attachment(this->textures["normal"]->handle),
                make_attachment(this->textures["depth"]->handle)
            });

    // LIGHT
    make_view(this->view_light, this->target_size);
    this->framebuffers["light"] =
        make_framebuffer(
            this->view_light,
            {
                make_attachment(this->textures["light"]->handle),
                make_attachment(this->textures["bloom"]->handle)
            });

    // LIGHT_BLUR
    make_view(this->view_bloom_blur, this->target_size);
    this->framebuffers["bloom_blur"] =
        make_framebuffer(
            this->view_bloom_blur,
            { make_attachment(this->textures["bloom_blur"]->handle) });

    // SSAO
    make_view(this->view_ssao, this->target_size);
    this->framebuffers["ssao"] =
        make_framebuffer(
            this->view_ssao,
            { make_attachment(this->textures["ssao"]->handle) });

    // SSAO_BLUR
    make_view(this->view_ssao_blur, this->target_size);
    this->framebuffers["ssao_blur"] =
        make_framebuffer(
            this->view_ssao_blur,
            { make_attachment(this->textures["ssao_blur"]->handle) });

    // SUN
    make_view(this->view_sun, glm::vec2(this->sun.texture_size));
    this->framebuffers["sun"] =
        make_framebuffer(
            this->view_sun,
            { make_attachment(this->textures["sun_depth"]->handle) });

    // BLUR
    make_view(this->view_blur0, this->target_size);
    this->framebuffers["blur0"] =
        make_framebuffer(
            this->view_blur0,
            { make_attachment(this->textures["blur0"]->handle) });

    make_view(this->view_blur1, this->target_size);
    this->framebuffers["blur1"] =
        make_framebuffer(
            this->view_blur1,
            { make_attachment(this->textures["blur1"]->handle) });

    make_view(this->view_blur2, this->target_size);
    this->framebuffers["blur2"] =
        make_framebuffer(
            this->view_blur2,
            { make_attachment(this->textures["blur2"]->handle) });
}

Renderer::~Renderer() {
    if (!this->initialized) {
        return;
    }

    for (auto &[_, uniform] : this->uniforms) {
        uniform.reset();
    }

    for (auto &[_, program] : this->programs) {
        program.reset();
    }

    for (auto &[_, texture] : this->textures) {
        texture.reset();
    }

    for (auto &[_, fb] : this->framebuffers) {
        fb.reset();
    }

    this->primitive.reset();
    bgfx::shutdown();
}


void Renderer::prepare_frame() {
    auto old_size = this->target_size;
    this->target_size = state.platform.window->get_size();

    if (this->target_size != old_size) {
        bgfx::reset(
            this->target_size.x, this->target_size.y,
            get_reset_flags(*this));
        bgfx::setViewRect(0, 0, 0, this->target_size.x, this->target_size.y);
        util::log::print(
            "Display resized to " +
            glm::to_string(this->target_size));
    }

    bgfx::touch(this->view_main);
}

void Renderer::end_frame() {
    bgfx::frame();
}

void Renderer::composite(RenderFn render) {
    auto
        &composite = *this->programs["composite"],
        &light = *this->programs["light"],
        &ssao = *this->programs["ssao"],
        &blur = *this->programs["blur"];

    // screen-space camera
    auto ss_camera =
        util::OrthoCamera(
            glm::vec2(0.0f, 0.0f),
            glm::vec2(1.0f, 1.0f),
            glm::vec2(-1.0f, 1.0f));

    auto screen_quad =
        [&](const auto &f, auto &program, auto &view) {
            ss_camera.set_view_transform(view);
            this->primitive->render_quad(
                glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f),
                glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f),
                f, program, true, view,
                BGFX_STATE_WRITE_MASK);
        };

    // configure render order
    const auto order =
        util::make_array(
            this->view_gbuffer,
            this->view_sun,
            this->view_light,
            this->view_ssao,
            this->view_blur0,
            this->view_blur1,
            this->view_blur2,
            this->view_ssao_blur,
            this->view_bloom_blur,
            this->view_main);
    bgfx::setViewOrder(0, order.size(), &order[0]);

    // render to sun's depth buffer
    this->sun.camera.set_view_transform(this->view_sun);
    render(
        this->view_sun,
        BGFX_STATE_WRITE_Z
        | BGFX_STATE_DEPTH_TEST_LESS
        | BGFX_STATE_CULL_CCW);

    // render to deferred buffers
    this->look_camera->set_view_transform(this->view_gbuffer);
    render(this->view_gbuffer, 0);

    // render to light buffer
    this->sun.direction = glm::vec3(0.60f, -0.7f, -0.30f);
    this->sun.ambient = glm::vec3(0.4);
    this->sun.diffuse = glm::vec3(1.0);
    this->sun.set_uniforms(light);

    this->look_camera->set_uniforms("u_look_light", light);
    light.try_set("s_gbuffer", 0, *this->textures["gbuffer"]);
    light.try_set("s_normal", 1, *this->textures["normal"]);
    light.try_set("s_depth", 2, *this->textures["depth"]);
    light.try_set("s_sun", 3, *this->textures["sun_depth"]);
    light.try_set("s_noise", 4, *this->textures["noise"]);
    screen_quad([](){}, light, this->view_light);

    // blur bloom buffer
    blur.try_set("s_input", 0, *this->textures["bloom"]);
    blur.try_set("u_params", glm::vec4(1, glm::vec3(0)));
    screen_quad([](){}, blur, this->view_blur0);

    blur.try_set("s_input", 0, *this->textures["blur0"]);
    blur.try_set("u_params", glm::vec4(0, glm::vec3(0)));
    screen_quad([](){}, blur, this->view_bloom_blur);

    // render to SSAO buffer
    // TODO: configurable count
    auto rand = util::rand(0x5540);
    std::array<glm::vec4, 64> ssao_samples;
    for (usize i = 0; i < ssao_samples.size(); i++) {
        const auto s =
            glm::lerp(
                0.1f, 1.0f,
                glm::pow(
                    i / static_cast<f32>(ssao_samples.size()),
                    2.0f));
        ssao_samples[i] =
            glm::vec4(
                s * glm::normalize(
                    glm::vec3(
                        rand.next<f32>(-1.0f, 1.0f),
                        rand.next<f32>(-1.0f, 1.0f),
                        rand.next<f32>(0.0f, 1.0f))),
                1.0);
    }

    ssao.try_set("ssao_samples", ssao_samples, ssao_samples.size());
    ssao.try_set("s_normal", 0, *this->textures["normal"]);
    ssao.try_set("s_depth", 1, *this->textures["depth"]);
    ssao.try_set("s_noise", 2, *this->textures["noise"]);
    this->look_camera->set_uniforms("u_look_ssao", ssao);
    screen_quad([](){}, ssao, this->view_ssao);

    // blur ssao buffer
    blur.try_set("s_input", 0, *this->textures["ssao"]);
    blur.try_set("u_params", glm::vec4(1, glm::vec3(0)));
    screen_quad([](){}, blur, this->view_blur1);

    blur.try_set("s_input", 0, *this->textures["blur1"]);
    blur.try_set("u_params", glm::vec4(0, glm::vec3(0)));
    screen_quad([](){}, blur, this->view_ssao_blur);

    // composite to main
    this->look_camera->set_uniforms("u_look", composite);
    composite.try_set("u_sky_color", Sky::COLORS[0][0]);
    composite.try_set("u_fog_color", Sky::COLORS[0][1]);
    composite.try_set("u_void_color", Sky::COLORS[0][2]);
    composite.try_set("u_fog", glm::vec4(128, 144, 0.0, 0.0));
    screen_quad(
        [&]() {
            composite.try_set("u_ticks", glm::vec4(state.time.ticks));
            composite.try_set("s_gbuffer", 0, *this->textures["gbuffer"]);
            composite.try_set("s_normal", 1, *this->textures["normal"]);
            composite.try_set("s_depth", 2, *this->textures["depth"]);
            composite.try_set("s_sun", 3, *this->textures["sun_depth"]);
            composite.try_set("s_noise", 4, *this->textures["noise"]);
            composite.try_set("s_ssao", 5, *this->textures["ssao_blur"]);
            composite.try_set("s_light", 6, *this->textures["light"]);
            composite.try_set("s_bloom", 7, *this->textures["bloom_blur"]);
        }, composite, this->view_main);
}
