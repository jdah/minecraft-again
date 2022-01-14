#include "util/log.hpp"

#include "state.hpp"

void util::log::print(
    const std::string &out,
    Level level,
    const char *_file,
    const char *_caller,
    const usize _line) {
    auto level_name =
        ((std::string[]){ "NORMAL", "WARN", "ERROR", "DEBUG" })[level];

    std::string f = _file;
    f = std::regex_replace(f, std::regex("src/"), "");
    f = std::regex_replace(f, std::regex("\\.(cpp|hpp)"), "");

    auto res = "[" + level_name + "]"
        + "[" + f
        + ":" + std::to_string(_line)
        + "]["+ _caller + "] "
        + out;

    if (res[res.length() - 1] != '\n') {
        res += '\n';
    }

    auto
        &serr = state.platform.log_err ? *state.platform.log_err : std::cerr,
        &sout = state.platform.log_out ? *state.platform.log_out : std::cout;

    (level == Level::ERROR ? serr : sout) << res;
}
