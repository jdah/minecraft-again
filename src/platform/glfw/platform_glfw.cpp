#include "platform/glfw/platform_glfw.hpp"
#include "gfx/gfx.hpp"
#include "state.hpp"
#include <optional>
#include <string>

using namespace platform::GLFW;

static void error_callback(int err, const char *msg) {
    util::log::print(
        "GLFW ERROR (" + std::to_string(err) + "): " + msg,
        util::log::ERROR);
}

Window::Window(glm::vec2 size, const std::string &title) {
    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) {
        std::cerr << "Error initializing GLFW" << std::endl;
        std::exit(1);
    }

    int num_monitors;
    GLFWmonitor **monitors = glfwGetMonitors(&num_monitors);
    GLFWmonitor *monitor = monitors[0];

    auto monitor_index =
        state.platform.settings["gfx"]["monitor"].value_or(0);

    if (monitor_index >= num_monitors) {
        util::log::print(
            "No such monitor " + std::to_string(monitor_index),
            util::log::Level::WARN);
    } else {
        monitor = monitors[monitor_index];
    }

    const GLFWvidmode *video_mode = glfwGetVideoMode(monitor);

    glm::ivec2 monitor_pos;
    glfwGetMonitorPos(monitor, &monitor_pos.x, &monitor_pos.y);

    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    this->window =
        glfwCreateWindow(
            size.x, size.y,
            title.c_str(), nullptr, nullptr);


    if (!window) {
        std::cerr << "Error creating window" << std::endl;
        glfwTerminate();
        std::exit(1);
    }

    // center and show window
    glfwDefaultWindowHints();
    glfwSetWindowPos(
        this->window,
        monitor_pos.x + (video_mode->width - size.x) / 2,
        monitor_pos.y + (video_mode->height - size.y) / 2);
    glfwSetWindowUserPointer(this->window, this);
    glfwShowWindow(this->window);

    // glfw specific, cannot handle a separate render thread
    // tell bgfx not to create a separate render thread
    bgfx::renderFrame();
}

Window::~Window() {
    if (this->window == nullptr) {
        return;
    }

    glfwTerminate();
    std::exit(0);
}

void Window::set_platform_data(
        bgfx::PlatformData &platform_data
    ) {
    // TODO: change for other platforms
    platform_data.nwh =
        reinterpret_cast<void *>(glfwGetCocoaWindow(this->window));
}

void Window::prepare_frame() {
    glfwPollEvents();
}

void Window::end_frame() {

}

bool Window::is_close_requested() {
    return glfwWindowShouldClose(this->window);
}

void Window::close() {
    glfwSetWindowShouldClose(this->window, true);
}

glm::ivec2 Window::get_size() {
    glm::ivec2 res;
    glfwGetWindowSize(this->window, &res.x, &res.y);
    return res;
};

Keyboard::Keyboard(Window &window) {
    // TODO: check that another keyboard has not already been added
    window.keyboard = this;
    auto wrapper =
        [](GLFWwindow *w, int k, int s, int a, int m) {
            auto window = static_cast<Window *>(glfwGetWindowUserPointer(w));
            window->keyboard->callback(w, k, s, a, m);
        };
    glfwSetKeyCallback(window.window, wrapper);
}

util::Iterator<platform::Button*> Keyboard::get_buttons() {
    return util::iter_values(this->keys);
}

std::optional<platform::Button *> Keyboard::operator[](
    const std::string &name) {
    auto n = util::to_lower(name);
    return this->keys.contains(n) ?
            std::make_optional(&this->keys[n]) :
            std::nullopt;
}

void Keyboard::callback(
    UNUSED GLFWwindow *window,
    int key, int scancode, int action, UNUSED int mods) {
    if (action == GLFW_REPEAT) {
        return;
    }

    Button *button;

    auto glfw_name = glfwGetKeyName(key, scancode);

    // extra names not covered by glfwGetKeyName
    static std::unordered_map<int, std::string> names = {
        { GLFW_KEY_SPACE,         "space"         },
        { GLFW_KEY_ESCAPE,        "escape"        },
        { GLFW_KEY_ENTER,         "enter"         },
        { GLFW_KEY_TAB,           "tab"           },
        { GLFW_KEY_BACKSPACE,     "backspace"     },
        { GLFW_KEY_INSERT,        "insert"        },
        { GLFW_KEY_DELETE,        "delete"        },
        { GLFW_KEY_RIGHT,         "right"         },
        { GLFW_KEY_LEFT,          "left"          },
        { GLFW_KEY_DOWN,          "down"          },
        { GLFW_KEY_UP,            "up"            },
        { GLFW_KEY_PAGE_UP,       "page_up"       },
        { GLFW_KEY_PAGE_DOWN,     "page_down"     },
        { GLFW_KEY_HOME,          "home"          },
        { GLFW_KEY_END,           "end"           },
        { GLFW_KEY_CAPS_LOCK,     "caps_lock"     },
        { GLFW_KEY_SCROLL_LOCK,   "scroll_lock"   },
        { GLFW_KEY_NUM_LOCK,      "num_lock"      },
        { GLFW_KEY_PRINT_SCREEN,  "print_screen"  },
        { GLFW_KEY_PAUSE,         "pause"         },
        { GLFW_KEY_F1,            "f1"            },
        { GLFW_KEY_F2,            "f2"            },
        { GLFW_KEY_F3,            "f3"            },
        { GLFW_KEY_F4,            "f4"            },
        { GLFW_KEY_F5,            "f5"            },
        { GLFW_KEY_F6,            "f6"            },
        { GLFW_KEY_F7,            "f7"            },
        { GLFW_KEY_F8,            "f8"            },
        { GLFW_KEY_F9,            "f9"            },
        { GLFW_KEY_F10,           "f10"           },
        { GLFW_KEY_F11,           "f11"           },
        { GLFW_KEY_F12,           "f12"           },
        { GLFW_KEY_LEFT_SHIFT,    "left_shift"    },
        { GLFW_KEY_LEFT_CONTROL,  "left_control"  },
        { GLFW_KEY_LEFT_ALT,      "left_alt"      },
        { GLFW_KEY_LEFT_SUPER,    "left_super"    },
        { GLFW_KEY_RIGHT_SHIFT,   "right_shift"   },
        { GLFW_KEY_RIGHT_CONTROL, "right_control" },
        { GLFW_KEY_RIGHT_ALT,     "right_alt"     },
        { GLFW_KEY_RIGHT_SUPER,   "right_super"   },
    };

    auto name =
        glfw_name ?
            util::to_lower(glfw_name) :
            (names.contains(key) ? names[key] : std::to_string(scancode));

    if (this->keys.contains(name)) {
        button = &this->keys[name];
    } else {
        button = &(this->keys[name] = Button((key << 16) | scancode, name));
    }

    button->down = action != GLFW_RELEASE;
}

Mouse::Mouse(Window &window) : platform::Mouse() {
    this->window = &window;
    window.mouse = this;

    auto wrapper_pos =
        [](GLFWwindow *w, double x, double y) {
            static_cast<Window *>(glfwGetWindowUserPointer(w))
                ->mouse->callback_pos(w, x, y);
        };

    auto wrapper_button =
        [](GLFWwindow *w, int b, int a, int m) {
            static_cast<Window *>(glfwGetWindowUserPointer(w))
                ->mouse->callback_button(w, b, a, m);
        };

    auto wrapper_enter =
        [](GLFWwindow *w, int e) {
            static_cast<Window *>(glfwGetWindowUserPointer(w))
                ->mouse->callback_enter(w, e);
        };

    auto wrapper_scroll =
        [](GLFWwindow *w, double x, double y) {
            static_cast<Window *>(glfwGetWindowUserPointer(w))
                ->mouse->callback_scroll(w, x, y);
        };

    glfwSetCursorPosCallback(window.window, wrapper_pos);
    glfwSetMouseButtonCallback(window.window, wrapper_button);
    glfwSetCursorEnterCallback(window.window, wrapper_enter);
    glfwSetScrollCallback(window.window, wrapper_scroll);
}

void Mouse::set_mode(platform::Mouse::Mode mode) {
    this->mode = mode;

    switch (mode) {
        case Mouse::DISABLED:
            glfwSetInputMode(
                this->window->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            break;
        case Mouse::HIDDEN:
            glfwSetInputMode(
                this->window->window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
            break;
        case Mouse::NORMAL:
            glfwSetInputMode(
                this->window->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            break;
    }
}

void Mouse::callback_pos(
    UNUSED GLFWwindow *window, double x, double y) {
    // NOTE: position is flipped! (0, 0) is bottom left
    this->pos = glm::vec2(x, this->window->get_size().y - y);
    this->pos_n =
        this->pos / glm::vec2(state.platform.window->get_size());
}

void Mouse::callback_button(
    UNUSED GLFWwindow *window, int button, int action, UNUSED int mods) {
    std::optional<Button *> b;

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        b = (*this)["left"];
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        b = (*this)["right"];
    } else if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
        b = (*this)["middle"];
    }

    if (b) {
        (*b)->down = action == GLFW_PRESS;
    }
}

void Mouse::callback_enter(
    UNUSED GLFWwindow *window, int entered) {
    this->in_window = static_cast<bool>(entered);
}

void Mouse::callback_scroll(
    UNUSED GLFWwindow *window, UNUSED double x, double y) {
    this->scroll += y;
}

