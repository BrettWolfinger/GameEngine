/// @file ConfigGroup.h
/// @brief Self-registering Field<T> and ConfigGroup for declarative game configs.
///
/// Define a config struct that inherits ConfigGroup and declares Field<T> members.
/// Call Application::registerConfig(path, &myConfig) once — the engine handles
/// TOML load/save, hot-reload, and the ImGui editor automatically.
///
/// Supported field types: float, int, bool, std::string.
#pragma once
#include <toml++/toml.h>
#include <string>
#include <vector>
#include <type_traits>

namespace Engine {

class ConfigGroup; // forward declare — needed by FieldBase constructor

// ─────────────────────────────── FieldBase ───────────────────────────────────

/// Type-erased base for all Field<T> instances.
/// Constructed by Field<T>; self-registers with the parent ConfigGroup.
class FieldBase {
public:
    virtual ~FieldBase() = default;

    const char* name() const { return m_name; }

    virtual void readFromToml(const toml::table& t) = 0;
    virtual void writeToToml(toml::table& t) const  = 0;

    bool isDirty()    const { return m_dirty; }
    void clearDirty()       { m_dirty = false; }

#ifdef ENABLE_TOOLS
    /// Render an ImGui widget for this field. No-op for unsupported types.
    virtual void renderImGui() = 0;
#endif

protected:
    /// Registers this field with @p parent. Defined in ConfigGroup.cpp where
    /// ConfigGroup is fully visible.
    FieldBase(ConfigGroup* parent, const char* name);
    const char* m_name;
    bool        m_dirty = false;
};

// ─────────────────────────────── Field<T> ────────────────────────────────────

/// A self-registering, implicitly-convertible config field.
///
/// Place Field<T> members inside a ConfigGroup subclass. Each member's
/// constructor automatically registers it for TOML I/O and ImGui editing.
///
/// @code
/// struct MyConfig : Engine::ConfigGroup {
///     Field<float> speed { this, "speed", 7.5f };
///     Field<int>   lives { this, "lives", 3    };
/// };
/// @endcode
template<typename T>
class Field : public FieldBase {
public:
    Field(ConfigGroup* parent, const char* name, T defaultValue)
        : FieldBase(parent, name), m_value(std::move(defaultValue)) {}

    /// Transparent read — use Field<T> wherever T is expected.
    operator T() const { return m_value; }

    /// Transparent write.
    Field& operator=(T v) { m_value = std::move(v); return *this; }

    /// Raw pointer access — required by ImGui widget functions.
    T* ptr() { return &m_value; }

    void readFromToml(const toml::table& t) override;
    void writeToToml(toml::table& t) const override;

#ifdef ENABLE_TOOLS
    void renderImGui() override;
#endif

private:
    T m_value;
};

// ─────────────────────────────── ConfigGroup ─────────────────────────────────

/// Base class for declarative config structs.
/// Field<T> members self-register on construction; the group drives I/O.
class ConfigGroup {
public:
    virtual ~ConfigGroup() = default;

    /// Called by Field<T> constructors — do not call manually.
    void registerField(FieldBase* field) { m_fields.push_back(field); }

    /// Populate all registered fields from a parsed TOML table.
    void readFromToml(const toml::table& t);

    /// Serialise all registered fields into a TOML table.
    void writeToToml(toml::table& t) const;

    bool isDirty() const {
        for (const auto* f : m_fields) if (f->isDirty()) return true;
        return false;
    }
    void clearDirty() { for (auto* f : m_fields) f->clearDirty(); }

#ifdef ENABLE_TOOLS
    /// Render a collapsing ImGui section for all registered fields.
    /// Returns true if the header is open (so callers can append extra widgets).
    bool renderImGui(const std::string& label);
#endif

private:
    std::vector<FieldBase*> m_fields;
};

// ──────────────────────── Field<T> inline implementations ────────────────────
// readFromToml and writeToToml only need toml++, which is already included.

template<typename T>
void Field<T>::readFromToml(const toml::table& t) {
    if constexpr (std::is_same_v<T, float>) {
        if (auto v = t[m_name].template value<double>())      m_value = static_cast<float>(*v);
    } else if constexpr (std::is_same_v<T, int>) {
        if (auto v = t[m_name].template value<int64_t>())     m_value = static_cast<int>(*v);
    } else if constexpr (std::is_same_v<T, bool>) {
        if (auto v = t[m_name].template value<bool>())        m_value = *v;
    } else if constexpr (std::is_same_v<T, std::string>) {
        if (auto v = t[m_name].template value<std::string>()) m_value = *v;
    }
}

template<typename T>
void Field<T>::writeToToml(toml::table& t) const {
    if constexpr (std::is_same_v<T, float>) {
        t.insert_or_assign(m_name, static_cast<double>(m_value));
    } else if constexpr (std::is_same_v<T, int>) {
        t.insert_or_assign(m_name, static_cast<int64_t>(m_value));
    } else if constexpr (std::is_same_v<T, bool>) {
        t.insert_or_assign(m_name, m_value);
    } else if constexpr (std::is_same_v<T, std::string>) {
        t.insert_or_assign(m_name, m_value);
    }
}

#ifdef ENABLE_TOOLS
// Primary template — no-op fallback for unsupported types.
// Explicit specialisations for float/int/bool/string live in ConfigGroup.cpp.
template<typename T>
void Field<T>::renderImGui() {}

template<> void Field<float>::renderImGui();
template<> void Field<int>::renderImGui();
template<> void Field<bool>::renderImGui();
template<> void Field<std::string>::renderImGui();
#endif

} // namespace Engine
