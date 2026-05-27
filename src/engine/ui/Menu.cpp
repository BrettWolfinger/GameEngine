#include "Menu.h"
#include <engine/core/Input.h>
#include <engine/renderer/PixelFont.h>
#include <GLFW/glfw3.h>
#include <string>

namespace Engine {

Menu::Menu(std::vector<std::string> items, float fontScale)
    : m_items(std::move(items))
    , m_fontScale(fontScale)
{}

void Menu::update() {
    m_confirmed = false;

    if (Input::isKeyPressed(GLFW_KEY_UP) || Input::isKeyPressed(GLFW_KEY_W)) {
        m_index = (m_index - 1 + static_cast<int>(m_items.size())) % static_cast<int>(m_items.size());
    }
    if (Input::isKeyPressed(GLFW_KEY_DOWN) || Input::isKeyPressed(GLFW_KEY_S)) {
        m_index = (m_index + 1) % static_cast<int>(m_items.size());
    }
    if (Input::isKeyPressed(GLFW_KEY_ENTER) || Input::isKeyPressed(GLFW_KEY_KP_ENTER)) {
        m_confirmed = true;
    }
}

void Menu::draw(Renderer2D& r, float x, float y, int layer) const {
    const float glyphH    = 7.f * m_fontScale;
    const float cursorW   = 6.f * m_fontScale;
    const float cursorPad = 4.f;

    for (int i = 0; i < static_cast<int>(m_items.size()); ++i) {
        const bool       selected = (i == m_index);
        const glm::vec4& color    = selected ? m_colorHighlighted : m_colorNormal;
        const float      itemY    = y + i * (glyphH + m_spacing);

        if (selected) {
            std::string cur(1, m_cursor);
            PixelFont::drawString(r, cur, x, itemY, m_fontScale, color, layer);
        }

        PixelFont::drawString(r, m_items[i], x + cursorW + cursorPad, itemY, m_fontScale, color, layer);
    }
}

void Menu::reset() {
    m_index     = 0;
    m_confirmed = false;
}

void Menu::setColors(const glm::vec4& normal, const glm::vec4& highlighted) {
    m_colorNormal      = normal;
    m_colorHighlighted = highlighted;
}

void Menu::setItemSpacing(float pixels) {
    m_spacing = pixels;
}

void Menu::setCursorGlyph(char glyph) {
    m_cursor = glyph;
}

} // namespace Engine
