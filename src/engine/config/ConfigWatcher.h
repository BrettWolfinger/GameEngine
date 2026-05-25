#pragma once
#ifndef ENGINE_INTERNAL
#  error "engine/config/ConfigWatcher.h is an engine-internal header. Include <engine/Engine.h> instead."
#endif
#include <filesystem>
#include <functional>
#include <string>
#include <vector>

namespace Engine {

class ConfigWatcher {
public:
    void watch(std::string_view path, std::function<void()> onChanged);

    // Called once per frame by Application. Checks each registered file for
    // changes and fires the callback if last_write_time has advanced.
    void poll();

private:
    struct Entry {
        std::string                          path;
        std::function<void()>                callback;
        std::filesystem::file_time_type      lastWrite;
    };

    std::vector<Entry> m_entries;
};

} // namespace Engine
