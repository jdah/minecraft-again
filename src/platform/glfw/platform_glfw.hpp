#ifndef PLATFORM_GLFW_HPP
#define PLATFORM_GLFW_HPP

#include "platform/platform.hpp"
#include "util/util.hpp"

// TODO: change for other platforms
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace platform::GLFW {
    // forward declaration
    struct Keyboard;
    struct Mouse;

    struct Window
        : platform::Window {
        GLFWwindow *window;
        Keyboard *keyboard;
        Mouse *mouse;

        Window(glm::vec2 size, const std::string& title);
        ~Window() override;

        Window(Window const &other) : window(other.window) {}

        Window(Window &&other) : window(other.window) {
            other.window = nullptr;
        }

        Window &operator=(Window const &other) {
            return *this = Window(other);
        }

        Window &operator=(Window &&other) {
            this->window = other.window;
            other.window = nullptr;
            return *this;
        }

        void set_platform_data(bgfx::PlatformData &platform_data) override;
        void prepare_frame() override;
        void end_frame() override;
        bool is_close_requested() override;
        void close() override;
        glm::ivec2 get_size() override;
    };

    // TODO: shift/"modifier" keys do not fully work. implement these by having
    // Keyboard::update use glfwGetKey with <LEFT/RIGHT>_SHIFT and manually
    // updating the values each frame
    struct Keyboard final
        : platform::Keyboard {
        // keys is originally this size and *must not* be changed
        static const constexpr usize MAX_KEYS = 1024;

        std::unordered_map<std::string, Button> keys;

        Keyboard(platform::GLFW::Window &window);
        util::Iterator<platform::Button *> get_buttons() override;
        std::optional<Button *> operator[](const std::string &name) override;
    private:
        void callback(
            GLFWwindow *window, int key, int scancode, int action, int mods);
    };

    struct Mouse final
        : platform::Mouse {
        Window *window;
        Mouse(platform::GLFW::Window &window);
        void set_mode(platform::Mouse::Mode mode) override;
    private:
        void callback_pos(
            GLFWwindow *window, double x, double y);
        void callback_button(
            GLFWwindow *window, int button, int action, int mods);
        void callback_enter(
            GLFWwindow *window, int entered);
        void callback_scroll(
            GLFWwindow *window, double x, double y);
    };
};

#endif
