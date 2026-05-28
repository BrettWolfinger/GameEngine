#include "ConfigRegistry.h"
#include "ConfigWatcher.h"
#include "ConfigLoader.h"
#include <toml++/toml.h>
#include <filesystem>
#include <fstream>

#ifdef ENABLE_TOOLS
#include <imgui.h>
#endif

namespace Engine {

ConfigRegistry::ConfigRegistry(ConfigWatcher& watcher)
    : m_watcher(watcher) {}

void ConfigRegistry::registerConfig(std::string_view path, ConfigGroup* group) {
    namespace fs = std::filesystem;
    std::string pathStr(path);
    std::string label = fs::path(pathStr).stem().string();

    if (fs::exists(pathStr)) {
        try {
            auto table = ConfigLoader::load(pathStr);
            group->readFromToml(table);
        } catch (...) {}
    } else {
        // Auto-generate TOML from Field<T> defaults — struct is the source of truth.
        toml::table table;
        group->writeToToml(table);
        fs::create_directories(fs::path(pathStr).parent_path());
        std::ofstream out(pathStr);
        if (out) out << table;
    }

    m_watcher.watch(pathStr, [group, pathStr]() {
        try {
            auto table = ConfigLoader::load(pathStr);
            group->readFromToml(table);
        } catch (...) {}
    });

    m_entries.push_back({pathStr, label, group});
}

void ConfigRegistry::saveEntry(const Entry& entry) {
    toml::table table;
    entry.group->writeToToml(table);
    std::ofstream out(entry.path);
    if (out) out << table;
}

#ifdef ENABLE_TOOLS
void ConfigRegistry::renderImGuiEditor() {
    ImGui::Begin("Config Editor");
    for (auto& entry : m_entries) {
        if (entry.group->renderImGui(entry.label)) {
            ImGui::Spacing();
            if (ImGui::Button(("Save##" + entry.label).c_str()))
                saveEntry(entry);
            ImGui::Spacing();
        }
    }
    ImGui::End();
}
#endif

} // namespace Engine
