/// @file ConfigLoader.h
/// @brief Loads a TOML config file from disk and returns the parsed table.
///
/// Game code calls ConfigLoader::load() once at startup (or hot-reload) and
/// passes the resulting table to the per-config fromToml() function.
/// Throws std::runtime_error on missing file or parse failure.
#pragma once
#include <toml++/toml.h>
#include <string_view>

namespace Engine {

class ConfigLoader {
public:
    /// Parse a TOML file at @p path and return the root table.
    /// @throws std::runtime_error on I/O or parse failure.
    static toml::table load(std::string_view path);
};

} // namespace Engine
