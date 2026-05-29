#include "ConfigGroup.h"

#ifdef ENABLE_TOOLS
#include <imgui.h>
#include <cstring>
#endif

namespace Engine {

// ─────────────────────────────── FieldBase ───────────────────────────────────

FieldBase::FieldBase(ConfigGroup* parent, const char* name)
    : m_name(name)
{
    parent->registerField(this);
}

// ─────────────────────────────── ConfigGroup ─────────────────────────────────

void ConfigGroup::readFromToml(const toml::table& t) {
    for (auto* field : m_fields)
        field->readFromToml(t);
    clearDirty();
}

void ConfigGroup::writeToToml(toml::table& t) const {
    for (const auto* field : m_fields)
        field->writeToToml(t);
}

#ifdef ENABLE_TOOLS

bool ConfigGroup::renderImGui(const std::string& label) {
    const std::string headerLabel = isDirty() ? label + " *" : label;
    const bool open = ImGui::CollapsingHeader(headerLabel.c_str(), ImGuiTreeNodeFlags_DefaultOpen);
    if (open) {
        for (auto* field : m_fields)
            field->renderImGui();
    }
    return open;
}

// ──────────────── Field<T>::renderImGui explicit specialisations ──────────────

template<>
void Field<float>::renderImGui() {
    const std::string lbl = m_dirty ? std::string(m_name) + " *" : m_name;
    if (ImGui::DragFloat(lbl.c_str(), &m_value, 0.1f))
        m_dirty = true;
}

template<>
void Field<int>::renderImGui() {
    const std::string lbl = m_dirty ? std::string(m_name) + " *" : m_name;
    if (ImGui::DragInt(lbl.c_str(), &m_value))
        m_dirty = true;
}

template<>
void Field<bool>::renderImGui() {
    const std::string lbl = m_dirty ? std::string(m_name) + " *" : m_name;
    if (ImGui::Checkbox(lbl.c_str(), &m_value))
        m_dirty = true;
}

template<>
void Field<std::string>::renderImGui() {
    char buf[256] = {};
    std::strncpy(buf, m_value.c_str(), sizeof(buf) - 1);
    const std::string lbl = m_dirty ? std::string(m_name) + " *" : m_name;
    if (ImGui::InputText(lbl.c_str(), buf, sizeof(buf))) {
        m_value = buf;
        m_dirty = true;
    }
}

#endif // ENABLE_TOOLS

} // namespace Engine
