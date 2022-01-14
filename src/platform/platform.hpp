#ifndef PLATFORM_HPP
#define PLATFORM_HPP

#include <toml.hpp>

#include "util/util.hpp"
#include "gfx/gfx.hpp"

#include "platform/input.hpp"

namespace platform {
    struct Window {
        virtual ~Window() { };
        virtual void set_platform_data(
                bgfx::PlatformData &platform_data
            ) = 0;
        virtual void prepare_frame() = 0;
        virtual void end_frame() = 0;
        virtual bool is_close_requested() = 0;
        virtual void close() = 0;
        virtual glm::ivec2 get_size() = 0;
    };

    struct Platform
        : util::Updateable, util::Tickable {
        std::string resources_path;
        std::ostream *log_out, *log_err;
        toml::table settings;
        std::unique_ptr<Window> window;
        std::unordered_map<std::string, std::unique_ptr<Input>> inputs;

        void update() override;
        void tick() override;

        // get inputs by name
        template <typename T>
        inline T &get_input(const std::string &name = "__USE_TYPENAME") {
            std::string n = name;

            if (name == "__USE_TYPENAME") {
                n = util::demangle(typeid(T).name());

                if (n.find(":") != std::string::npos) {
                    n = n.substr(n.rfind(":") + 1, n.length());
                }
            }

            return *dynamic_cast<T *>(inputs[util::to_lower(n)].get());
        }
    };
};

#endif
