#ifndef UTIL_ARENA_HPP
#define UTIL_ARENA_HPP

#include "util/std.hpp"
#include "util/types.hpp"
#include "util/assert.hpp"

namespace util {
struct Bump {
    u8 *mem, *cur, *end;

    Bump() = default;
    Bump(const Bump &other) = delete;
    Bump(Bump &&other) { *this = std::move(other); }
    Bump &operator=(const Bump &other) = delete;
    Bump &operator=(Bump &&other) {
        this->mem = other.mem;
        this->cur = other.cur;
        this->end = other.end;
        other.mem = nullptr;
        return *this;
    }

    Bump(usize n) {
        this->mem = reinterpret_cast<u8*>(std::malloc(n));
        this->cur = this->mem;
        this->end = this->mem + n;
    }

    ~Bump() {
        if (this->mem) {
            std::free(this->mem);
        }
    }

    inline void *alloc(usize n) {
        util::_assert(this->mem, "Bump allocator not initialized");

        usize align =
            reinterpret_cast<usize>(this->cur) % 16 != 0 ?
                16 - (reinterpret_cast<usize>(this->cur) % 16)
                : 0;

        util::_assert(
            this->cur + align + n <= this->end,
            "Bump allocator at "
                + std::to_string(reinterpret_cast<usize>(this))
                + " out of memory!");

        void *res = this->cur + align;
        this->cur += align + n;
        return res;
    }

    template <typename T>
    inline T *alloc() {
        return reinterpret_cast<T *>(alloc(sizeof(T)));
    }

    template <typename T>
    inline T *alloc_copy(T *src) {
        T *res = reinterpret_cast<T *>(alloc(sizeof(T)));
        *res = *src;
        return *res;
    }

    inline void clear() {
        this->cur = this->mem;
    }
};
}

#endif
