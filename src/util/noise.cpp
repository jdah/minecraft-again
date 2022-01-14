#include "util/noise.hpp"

using namespace util;

extern "C" {
    #include <noise1234.h>
}

f32 Octave::sample(glm::vec2 i) const {
    f32 u = 1.0f, v = 0.0f;
    for (usize j = 0; j < this->n; j++) {
        v += noise3(i.x / u, i.y / u, this->seed + j + (this->o * 32)) * u;
        u *= 2.0f;
    }
    return v;
}

f32 Combined::sample(glm::vec2 i) const {
    return this->n->sample(glm::vec2(i.x + this->m->sample(i), i.y));
}
