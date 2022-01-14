#ifndef UTIL_TYPES_HPP
#define UTIL_TYPES_HPP

#include <cstdint>
#include <cstddef>
#include <cinttypes>
#include <cstdlib>
#include <vector>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef size_t usize;
typedef intmax_t ssize;

typedef float f32;
typedef double f64;

namespace util {
class Updateable {
public:
    virtual void update() = 0;
};


class Tickable {
public:
    virtual void tick() = 0;
};

// template helpers
template <class T>
struct is_vector {
    static const bool value = false;
};

template <class T>
struct is_vector<std::vector<T>> {
    static const bool value = true;
};

// various other helpers
// from stackoverflow.com/questions/26351587/how-to-create-stdarray-with-initialization-list-without-providing-size-directl
template <typename... T>
constexpr auto make_array(T&&... values) ->
    std::array<
       typename std::decay<
           typename std::common_type<T...>::type>::type,
       sizeof...(T)> {
    return std::array<
        typename std::decay<
            typename std::common_type<T...>::type>::type,
        sizeof...(T)>{std::forward<T>(values)...};
}

// wrapper for T which is non-copyable
// when moved, other is initialized according to default constructor
template <typename T>
struct Moveable {
    Moveable() = default;
    Moveable(const Moveable<T> &other) = delete;
    Moveable(Moveable<T> &&other) {
        *this = std::move(other);
    }
    Moveable &operator=(const Moveable<T> &other) = delete;
    inline Moveable &operator=(Moveable<T> &&other) {
        this->t = other.t;
        other.t = T();
        return *this;
    }

    inline Moveable &operator=(const T &t) {
        this->t = t;
        return *this;
    }

    inline operator T() {
        return this->t;
    }
private:
    T t = T();
};
}

// potentially compiler-specific macros
#define UNUSED __attribute__((unused))

#define STRINGIFY(x) #x
#define PRAGMA(x) _Pragma(#x)

#if defined(__clang__)
#define UNROLL(n) PRAGMA(clang loop unroll_count(n))
#elif defined(__GNU__)
#define UNROLL(n) PRAGMA(GCC unroll n)
#else
#define UNROLL(n) PRAGMA(unroll)
#endif

#endif
