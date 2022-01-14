#ifndef UTIL_FILE_HPP
#define UTIL_FILE_HPP

#include <optional>
#include <string>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <array>
#include <vector>
#include <iterator>

#include "util/types.hpp"
#include "util/result.hpp"

namespace util {
    static inline util::Result<std::vector<std::string>, std::string>
        list_files(const std::string &path) {
        if (!std::filesystem::is_directory(path)) {
            return util::Err(path + " is not a directory");
        }

        std::vector<std::string> res;

        try {
            for (auto const &e : std::filesystem::directory_iterator(path)) {
                res.push_back(e.path().generic_string());
            }
        } catch (const std::exception &e) {
            return util::Err(std::string(e.what()));
        }

        return util::Ok(res);
    }

    static inline util::Result<bool, std::string> is_directory(
        const std::string &path) {
        if (std::filesystem::exists(path)) {
            return util::Ok(std::filesystem::is_directory(path));
        } else {
            return util::Err(path + " does not exist");
        }
    }

    static inline util::Result<std::string, std::string> read_file(
        const std::string &path) {
        std::ifstream input(path);

        if (!input.good()) {
            return util::Err("Error opening file " + path);
        }

        std::stringstream buf;
        buf << input.rdbuf();

        if (input.fail()) {
            return util::Err("Error reading file " + path);
        }

        return util::Ok(buf.str());
    }

    // splits a path into { base, filename, ext }
    static inline std::tuple<std::string, std::string, std::string> split_path(
        const std::string &path) {
        auto last_sep = path.find_last_of("/\\"),
             ext_sep = path.find_last_of(".");

        const auto base =
            last_sep == std::string::npos ?
                ""
                : path.substr(0, last_sep);

        const auto filename =
            last_sep == std::string::npos ?
                path.substr(0, ext_sep)
                : path.substr(
                    last_sep + 1,
                    ext_sep == std::string::npos ?
                        std::string::npos
                        : (ext_sep - (last_sep + 1)));

        const auto ext =
            ext_sep == std::string::npos ?
                ""
                : path.substr(ext_sep + 1);

        return { base, filename, ext };
    }
}

#endif
