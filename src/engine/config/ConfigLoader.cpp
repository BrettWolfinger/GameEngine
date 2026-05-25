#include "ConfigLoader.h"
#include <stdexcept>
#include <string>

namespace Engine {

toml::table ConfigLoader::load(std::string_view path) {
    try {
        return toml::parse_file(path);
    } catch (const toml::parse_error& e) {
        throw std::runtime_error(
            std::string("ConfigLoader: '") + std::string(path) + "': " + e.what()
        );
    }
}

} // namespace Engine
