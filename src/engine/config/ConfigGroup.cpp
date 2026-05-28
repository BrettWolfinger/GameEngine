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
}

void ConfigGroup::writeToToml(toml::table& t) const {
    for (const auto* field : m_fields)
        field->writeToToml(t);
}

#ifdef ENABLE_TOOLS

void ConfigGroup::renderImGui(const std::string& label) {
    if (ImGui::CollapsingHeader(label.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
        for (auto* field : m_fields)
            field->renderImGui();
    }
}

// ──────────────── Field<T>::renderImGui explicit specialisations ──────────────

template<>
void Field<float>::renderImGui() {
    ImGui::DragFloat(m_name, &m_value, 0.1f);
}

template<>
void Field<int>::renderImGui() {
    ImGui::DragInt(m_name, &m_value);
}

template<>
void Field<bool>::renderImGui() {
    ImGui::Checkbox(m_name, &m_value);
}

template<>
void Field<std::string>::renderImGui() {
    char buf[256] = {};
    std::strncpy(buf, m_value.c_str(), sizeof(buf) - 1);
    if (ImGui::InputText(m_name, buf, sizeof(buf)))
        m_value = buf;
}

#endif // ENABLE_TOOLS

} // namespace Engine
