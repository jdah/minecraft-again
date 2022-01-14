#ifndef PLATFORM_INPUT_HPP
#define PLATFORM_INPUT_HPP

#include "util/util.hpp"

namespace platform {
    struct Button {
        usize id;
        std::string name;
        bool down, last, last_tick, pressed, pressed_tick;

        Button() = default;
        Button(usize id, std::string name)
            : id(id), name(name) { };

        inline void update() {
            this->pressed = this->down && !this->last;
            this->last = this->down;
        }

        inline void tick() {
            this->pressed_tick = this->down && !this->last_tick;
            this->last_tick = this->down;
        }
    };

    struct Input
        : util::Updateable, util::Tickable {
        virtual ~Input() = default;
        void update() override;
        void tick() override;

        virtual util::Iterator<Button*> get_buttons();
    };

    // TODO: proper text input (on GLFW, see glfwSetCharCallback)
    struct Keyboard
        : Input {

        // retrieve a key by name
        virtual std::optional<Button*> operator[](const std::string &name) = 0;
    };

    struct Mouse
        : Input {
        enum Mode {
            DISABLED,
            HIDDEN,
            NORMAL
        };

        enum Index {
            LEFT = 0,
            RIGHT = 1,
            MIDDLE = 2
        };

        Mode mode;
        std::vector<Button> buttons;

        // mouse position, not normalized
        glm::vec2 pos, pos_delta, pos_delta_tick;

        // normalized (to window size) positions
        glm::vec2 pos_n, pos_delta_n, pos_delta_tick_n;

        // scroll distance
        f32 scroll, scroll_delta, scroll_delta_tick;

        // true when mouse is contained in window
        bool in_window;

        virtual void set_mode(Mode mode) = 0;

        Mouse();
        util::Iterator<Button*> get_buttons() override;
        void update() override;
        void tick() override;

        // built-in access for left/right/middle
        virtual std::optional<Button*> operator[](
            const std::string &name);

    private:
        glm::vec2 last_pos, last_pos_tick;
        f32 last_scroll, last_scroll_tick;
    };
};

#endif
