/// @file Config.h
/// @brief Engine config facade — register hot-reload callbacks for config files.
///
/// Hot-reload is active in debug builds only (ENABLE_TOOLS). In release
/// builds watch() is a no-op, so game code needs no preprocessor guards.
#pragma once
#include <functional>
#include <string_view>

namespace Engine::Config {

/// Register @p onChanged to be called whenever the file at @p path is
/// modified on disk. No-op in release builds.
void watch(std::string_view path, std::function<void()> onChanged);

} // namespace Engine::Config
