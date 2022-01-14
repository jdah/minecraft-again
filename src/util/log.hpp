#ifndef UTIL_LOG_HPP
#define UTIL_LOG_HPP

#include <string>
#include <sstream>
#include <type_traits>
#include <experimental/type_traits>

#include "util/types.hpp"
#include "util/math.hpp"

namespace util::log {
    enum Level {
        NORMAL,
        WARN,
        ERROR,
        DEBUG,
        DEFAULT = DEBUG
    };

    void print(
        const std::string &out,
        Level level = Level::DEFAULT,
        const char *_file = __builtin_FILE(),
        const char *_caller = __builtin_FUNCTION(),
        const usize _line = __builtin_LINE());

    // std::to_string-able types
    template <typename T>
        requires (!std::is_convertible<T, std::string>::value &&
                  !std::is_same<T, std::stringstream>::value)
    auto print(
        const T &out,
        Level level = Level::DEFAULT,
        const char *_file = __builtin_FILE(),
        const char *_caller = __builtin_FUNCTION(),
        const usize _line = __builtin_LINE())
            -> decltype(std::to_string(out), void()) {
        print(std::to_string(out), level, _file, _caller, _line);
    }

    inline void print(
        const std::stringstream &out,
        Level level = Level::DEFAULT,
        const char *_file = __builtin_FILE(),
        const char *_caller = __builtin_FUNCTION(),
        const usize _line = __builtin_LINE()) {
        print(out.str(), level, _file, _caller, _line);
    }

    // dummy struct to indicate stream end
    struct _End {};
    static constexpr auto end = _End();

    // log instance which behaves like a stringstream
    struct _Log {
        _Log(
            const char *file,
            const char *caller,
            const usize line)
            : file(file),
              caller(caller),
              line(line),
              level(Level::DEFAULT) {}

        template <typename T>
            requires (util::is_mat<T>::value || util::is_vec<T>::value)
        _Log &operator<<(T const &rhs) {
            this->ss << glm::to_string(rhs);
            return *this;
        }

        template <typename T>
            requires (!util::is_mat<T>::value
                      && !util::is_vec<T>::value
                      && !std::is_same_v<T, _End>
                      && !std::is_same_v<T, Level>)
        _Log &operator<<(T const &rhs) {
            this->ss << rhs;
            return *this;
        }

        // invoked when called with util::log::end
        template <typename T>
            requires std::is_same_v<T, _End>
        void operator<<(T const &_) {
            util::log::print(
                this->ss, this->level,
                this->file, this->caller, this->line);
        }

        template <typename T>
            requires std::is_same_v<T, Level>
        _Log &operator<<(T const &rhs) {
            this->level = rhs;
            return *this;
        }

    private:
        const char *file, *caller;
        usize line;
        Level level;
        std::stringstream ss;
    };

    // returns a logger which behaves like an std::stringstream
    // must be terminated (printed) with util::log::end
    static inline _Log out(
        const char *_file = __builtin_FILE(),
        const char *_caller = __builtin_FUNCTION(),
        const usize _line = __builtin_LINE()) {
        return _Log(_file, _caller, _line);
    }
}

#endif
