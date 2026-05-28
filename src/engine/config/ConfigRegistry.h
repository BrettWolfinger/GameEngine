#pragma once
#include "ConfigGroup.h"
#include <string_view>
#include <string>
#include <vector>

namespace Engine {

class ConfigWatcher;

/// Owns the collection of registered ConfigGroups and drives TOML
/// load/auto-generation, hot-reload watcher registration, and ImGui editing.
/// Constructed with a ConfigWatcher reference; Application::Impl holds one.
class ConfigRegistry {
public:
    explicit ConfigRegistry(ConfigWatcher& watcher);

    /// Register a ConfigGroup: loads TOML if present, otherwise auto-generates
    /// it from Field<T> defaults, then watches the file for hot-reload.
    void registerConfig(std::string_view path, ConfigGroup* group);

#ifdef ENABLE_TOOLS
    /// Render the "Config Editor" ImGui window containing all registered groups.
    void renderImGuiEditor();
#endif

private:
    struct Entry {
        std::string  path;
        std::string  label;  // file stem — used as ImGui CollapsingHeader label
        ConfigGroup* group;
    };

    ConfigWatcher&     m_watcher;
    std::vector<Entry> m_entries;
};

} // namespace Engine
