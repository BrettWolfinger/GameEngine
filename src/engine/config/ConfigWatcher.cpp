#include "ConfigWatcher.h"

namespace Engine {

void ConfigWatcher::watch(std::string_view path, std::function<void()> onChanged) {
    m_entries.push_back({
        std::string(path),
        std::move(onChanged),
        std::filesystem::last_write_time(path),
    });
}

void ConfigWatcher::poll() {
    for (auto& e : m_entries) {
        const auto lw = std::filesystem::last_write_time(e.path);
        if (lw != e.lastWrite) {
            e.lastWrite = lw;
            e.callback();
        }
    }
}

} // namespace Engine
