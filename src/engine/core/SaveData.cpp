#include "SaveData.h"
#include <fstream>
#include <filesystem>

namespace Engine {

SaveData SaveData::load(const std::string& slotName) {
    SaveData sd;
    sd.m_slotName = slotName;

    std::ifstream file("saves/" + slotName + ".sav");
    if (!file.is_open()) return sd;

    std::string line;
    while (std::getline(file, line)) {
        auto sep = line.find('=');
        if (sep == std::string::npos) continue;
        sd.m_entries[line.substr(0, sep)] = line.substr(sep + 1);
    }
    return sd;
}

void SaveData::save() const {
    std::filesystem::create_directories("saves");
    std::ofstream file("saves/" + m_slotName + ".sav");
    for (const auto& [key, value] : m_entries)
        file << key << '=' << value << '\n';
}

int SaveData::getInt(const std::string& key, int defaultValue) const {
    auto it = m_entries.find(key);
    if (it == m_entries.end()) return defaultValue;
    try { return std::stoi(it->second); }
    catch (...) { return defaultValue; }
}

float SaveData::getFloat(const std::string& key, float defaultValue) const {
    auto it = m_entries.find(key);
    if (it == m_entries.end()) return defaultValue;
    try { return std::stof(it->second); }
    catch (...) { return defaultValue; }
}

std::string SaveData::getString(const std::string& key, const std::string& defaultValue) const {
    auto it = m_entries.find(key);
    return it != m_entries.end() ? it->second : defaultValue;
}

void SaveData::setInt(const std::string& key, int value) {
    m_entries[key] = std::to_string(value);
}

void SaveData::setFloat(const std::string& key, float value) {
    m_entries[key] = std::to_string(value);
}

void SaveData::setString(const std::string& key, const std::string& value) {
    m_entries[key] = value;
}

} // namespace Engine
