#include "platform/platform.hpp"

using namespace platform;

void Platform::update() {
    for (auto &[_, input] : this->inputs) {
        input->update();
    }
}

void Platform::tick() {
    for (auto &[_, input] : this->inputs) {
        input->tick();
    }
}
