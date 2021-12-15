#include <iostream>
#include <cinttypes>
#include <cstdint>
#include <cstddef>
#include <string>
#include <sstream>
#include <fstream>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>

// TODO: move elsewhere
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef size_t usize;
typedef ssize_t isize;

typedef float f32;
typedef double f64;

static const glm::vec2 SIZE = glm::vec2(1280, 720);

struct Vertex {
    glm::vec3 position;
    u32 color;
};

static Vertex cube_vertices[] = {
   	{ glm::vec3(-1.0f,  1.0f,  1.0f), 0xff000000 },
	{ glm::vec3( 1.0f,  1.0f,  1.0f), 0xff0000ff },
	{ glm::vec3(-1.0f, -1.0f,  1.0f), 0xff00ff00 },
	{ glm::vec3( 1.0f, -1.0f,  1.0f), 0xff00ffff },
	{ glm::vec3(-1.0f,  1.0f, -1.0f), 0xffff0000 },
	{ glm::vec3( 1.0f, 1.0f, -1.0f), 0xffff00ff },
	{ glm::vec3(-1.0f, -1.0f, -1.0f), 0xffffff00 },
	{ glm::vec3( 1.0f, -1.0f, -1.0f), 0xffffffff },
};

static const u16 cube_indices[] = {
    0, 1, 2,
	1, 3, 2,
	4, 6, 5,
	5, 6, 7,
	0, 2, 4,
	4, 2, 6,
	1, 5, 3,
	5, 7, 3,
	0, 4, 1,
	4, 5, 1,
	2, 3, 6,
	6, 3, 7,
};

static bgfx::ShaderHandle load_shader(
        const std::string &base_path,
        const std::string &name,
        const std::string &file_name
    ) {
    std::stringstream path;
    std::string platform;

    switch (bgfx::getRendererType()) {
        case bgfx::RendererType::Noop:
        case bgfx::RendererType::Direct3D9:  platform = "dx9";   break;
        case bgfx::RendererType::Direct3D11:
        case bgfx::RendererType::Direct3D12: platform = "dx11";  break;
        case bgfx::RendererType::Agc:
        case bgfx::RendererType::Gnm:        platform = "pssl";  break;
        case bgfx::RendererType::Metal:      platform = "metal"; break;
        case bgfx::RendererType::Nvn:        platform = "nvn";   break;
        case bgfx::RendererType::OpenGL:     platform = "glsl";  break;
        case bgfx::RendererType::OpenGLES:   platform = "essl";  break;
        case bgfx::RendererType::Vulkan:     platform = "spirv"; break;
        case bgfx::RendererType::WebGPU:     platform = "spirv"; break;
        default:
            // TODO
            break;
    }

    path << base_path << "/" << name << "/" << platform << "/" << file_name;

    std::ifstream input(path.str());
    std::stringstream buf;
    buf << input.rdbuf();

    auto text = buf.str();
    auto result = bgfx::createShader(bgfx::copy(text.c_str(), text.length()));
    bgfx::setName(result, file_name.c_str());
    return result;
}

static bgfx::ProgramHandle load_program(
        const std::string &program_name,
        const std::string &vs_name,
        const std::string &fs_name
    ) {
    auto
        vs = load_shader("res/shaders", program_name, vs_name),
        fs = load_shader("res/shaders", program_name, fs_name);

    return bgfx::createProgram(vs, fs, true);
}

static void glfw_error_callback(int err, const char *msg) {
    std::cerr << "GLFW ERROR (" << err << "): " << msg << std::endl;
}

int main(int argc, char *argv[]) {
    glfwSetErrorCallback(glfw_error_callback);

    if (!glfwInit()) {
        std::cerr << "Error initializing GLFW" << std::endl;
        return -1;
    }

    GLFWwindow *window =
        glfwCreateWindow(
            SIZE.x, SIZE.y,
            "GAME", nullptr, nullptr);

    if (!window) {
        std::cerr << "Error creating window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // TODO: possibly GLFW/OSX specific
    // tell bgfx to not create a separate render thread
    bgfx::renderFrame();

    bgfx::Init init;

    bgfx::PlatformData platform_data;
    platform_data.nwh = glfwGetCocoaWindow(window);
    init.platformData = platform_data;

    // let system choose renderer
    init.type = bgfx::RendererType::Count;
    init.resolution.width = SIZE.x;
    init.resolution.height = SIZE.y;
    init.resolution.reset = BGFX_RESET_VSYNC;
    bgfx::init(init);

    bgfx::reset(SIZE.x, SIZE.y, BGFX_RESET_VSYNC, init.resolution.format);
    bgfx::setDebug(BGFX_DEBUG_TEXT);
    bgfx::setViewClear(
        0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x003030FF, 1.0f, 0);
    bgfx::setViewRect(0, 0, 0, bgfx::BackbufferRatio::Equal);

    bgfx::VertexLayout layout;
    layout.begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8)
        .end();

    bgfx::VertexBufferHandle vertex_buffer =
        bgfx::createVertexBuffer(
            bgfx::makeRef(cube_vertices, sizeof(cube_vertices)),
            layout);

    bgfx::IndexBufferHandle index_buffer =
        bgfx::createIndexBuffer(
            bgfx::makeRef(cube_indices, sizeof(cube_indices)));

    bgfx::ProgramHandle program =
        load_program("test", "vs_test.sc", "fs_test.sc");

    f32 time = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        bgfx::touch(0);

        bgfx::dbgTextClear();
        bgfx::dbgTextPrintf(0, 0, 0x6f, "Hello, world");

        glm::mat4 model, view, proj;
        view = glm::lookAt(
            glm::vec3(0.0f, 20.0f, -35.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f));
        proj = glm::perspective(
            glm::radians(85.0f), SIZE.x / SIZE.y, 0.01f, 100.0f);

        bgfx::setViewTransform(
            0,
            reinterpret_cast<void *>(&view),
            reinterpret_cast<void *>(&proj));
        bgfx::setViewRect(0, 0, 0, SIZE.x, SIZE.y);

        u64 state =
            BGFX_STATE_WRITE_MASK
            | BGFX_STATE_DEPTH_TEST_LESS
            | BGFX_STATE_CULL_CW
            | BGFX_STATE_MSAA
            | 0; // triangle list

        for (uint32_t yy = 0; yy < 11; ++yy) {
            for (uint32_t xx = 0; xx < 11; ++xx) {
                glm::mat4 rotate, translate;
                rotate =
                    glm::rotate(glm::mat4(1.0), time + xx * 0.21f, glm::vec3(1, 0, 0));
                rotate =
                    glm::rotate(rotate, time + yy * 0.37f, glm::vec3(0, 1, 0));
                translate =
                    glm::translate(
                        glm::mat4(1.0),
                        glm::vec3(-15.0f + (f32) xx * 3.0f, -15.0f + (f32) yy * 3.0f, 0.0f));

                model = translate * rotate;
                bgfx::setTransform(reinterpret_cast<void *>(&model));

                bgfx::setVertexBuffer(0, vertex_buffer);
                bgfx::setIndexBuffer(index_buffer);
                bgfx::setState(state);

                bgfx::submit(0, program);
            }
        }

        bgfx::frame();
        time += 0.01f;
    }

    bgfx::destroy(program);
    bgfx::destroy(vertex_buffer);
    bgfx::destroy(index_buffer);
    bgfx::shutdown();
    glfwTerminate();

    return 0;
}
