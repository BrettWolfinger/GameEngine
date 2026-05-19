#pragma once
#include <string>
#include <unordered_map>

namespace Engine {

class SaveData {
public:
    static SaveData load(const std::string& slotName);
    void save() const;

    int         getInt   (const std::string& key, int defaultValue = 0)                 const;
    float       getFloat (const std::string& key, float defaultValue = 0.f)             const;
    std::string getString(const std::string& key, const std::string& defaultValue = "") const;

    void setInt   (const std::string& key, int value);
    void setFloat (const std::string& key, float value);
    void setString(const std::string& key, const std::string& value);

private:
    std::string m_slotName;
    std::unordered_map<std::string, std::string> m_entries;
};

} // namespace Engine
