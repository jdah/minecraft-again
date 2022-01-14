#ifndef UTIL_TIME_HPP
#define UTIL_TIME_HPP

#include <algorithm>
#include <array>
#include <iomanip>
#include <type_traits>

#include "util/log.hpp"
#include "util/types.hpp"

namespace util {
struct Time {
    static constexpr u64
        NANOS_PER_SECOND   = 1000000000,
        NANOS_PER_MILLIS   = 1000000,
        MILLIS_PER_SECOND  = 1000,
        TICKS_PER_SECOND   = 60,
        NANOS_PER_TICK     = (NANOS_PER_SECOND / TICKS_PER_SECOND);

    template <typename T>
    static inline auto to_millis(T nanos) {
        return nanos / static_cast<T>(NANOS_PER_MILLIS);
    }

    template <typename T>
    static inline auto to_seconds(T nanos) {
        return nanos / static_cast<T>(NANOS_PER_SECOND);
    }

    struct Section {
        static constexpr usize RUNNING_AVG_LENGTH = 120;

        std::array<u64, RUNNING_AVG_LENGTH> times;
        u64 start;

        Section() = default;
        Section(Time *time, bool use_ticks = false)
            : time(time), use_ticks(use_ticks) {
            std::memset(&this->times, 0, sizeof(this->times));
        }

        inline void begin() {
            this->start = this->time->now();
        }

        inline void end() {
            this->times[
                (this->use_ticks ? this->time->ticks : this->time->frames)
                    % RUNNING_AVG_LENGTH] =
                this->time->now() - this->start;
        }

        inline f64 avg() {
            u64 sum = 0;
            for (auto i : this->times) {
                sum += i;
            }
            return sum / static_cast<f64>(RUNNING_AVG_LENGTH);
        }

    private:
        Time *time;
        bool use_ticks;
    } section_update, section_render, section_tick, section_frame;

    u64 time, last_frame, last_second;
    u64 ticks, second_ticks, tps, tick_remainder, frame_ticks;
    u64 frames, second_frames, fps;
    u64 delta;

    std::function<u64(void)> now;

    Time() = default;

    explicit Time(std::function<u64(void)> now) {
        this->init(now);
    }

    Time(const Time &other) : Time(other.now) {}

    Time &operator=(const Time &other) {
        this->init(other.now);
        return *this;
    }

    void update();

    template <typename F>
        requires std::is_invocable_v<F>
    inline void time_section(const std::string &name, F f) {
        const auto start = this->now();
        f();
        util::log::out()
            << "section " << name << ": "
            << std::fixed << std::setprecision(3)
            << to_millis<f64>(this->now() - start)
            << util::log::end;
    }
private:
    void init(std::function<u64(void)> now) {
        this->now = now;
         this->section_update = Section(this);
        this->section_render = Section(this);
        this->section_tick = Section(this, true);
        this->section_frame = Section(this);
        this->time = this->now();
        this->last_frame = this->now();
        this->last_second = this->now();
    }
};
}

#endif
