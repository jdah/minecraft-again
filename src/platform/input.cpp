#include "platform/input.hpp"
#include "state.hpp"

using namespace platform;

void Input::update() {
    for (auto &b : this->get_buttons()) {
        b->update();
    }
}

void Input::tick() {
    for (auto &b : this->get_buttons()) {
        b->tick();
    }
}

util::Iterator<Button*> Input::get_buttons() {
    return util::Iterator<Button*>();
}

Mouse::Mouse() {
    this->buttons = {
        { Index::LEFT, "left" },
        { Index::RIGHT, "right" },
        { Index::MIDDLE, "middle" },
    };
}

void Mouse::update() {
    Input::update();
    this->pos_delta = this->pos - this->last_pos;
    this->pos_delta_n =
        this->pos_delta / glm::vec2(state.platform.window->get_size());
    this->last_pos = this->pos;
    this->scroll_delta = this->scroll - this->last_scroll;
    this->last_scroll = this->scroll;
}

void Mouse::tick() {
    Input::tick();
    this->pos_delta_tick = this->pos - this->last_pos_tick;
    this->pos_delta_tick_n =
        this->pos_delta_tick / glm::vec2(state.platform.window->get_size());
    this->last_pos_tick = this->pos;
    this->scroll_delta_tick = this->scroll - this->last_scroll_tick;
    this->last_scroll_tick = this->scroll;
}

util::Iterator<Button*> Mouse::get_buttons() {
    return util::iter(this->buttons).ptr();
}

std::optional<Button *> Mouse::operator[](const std::string &name) {
    auto n = util::to_lower(name);

    if (n == "left") {
        return std::make_optional(&this->buttons[Index::LEFT]);
    } else if (n == "right") {
        return std::make_optional(&this->buttons[Index::RIGHT]);
    } else if (n == "middle") {
        return std::make_optional(&this->buttons[Index::MIDDLE]);
    }

    return std::nullopt;
};

