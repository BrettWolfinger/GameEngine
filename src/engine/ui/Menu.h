#pragma once
#include <engine/renderer/Renderer2D.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace Engine {

class Menu {
public:
    Menu() = default;
    explicit Menu(std::vector<std::string> items, float fontScale = 2.f);

    void update();
    void draw(Renderer2D& r, float x, float y) const;

    bool confirmed()    const { return m_confirmed; }
    int  selectedIndex() const { return m_index; }
    void reset();

    void setColors(const glm::vec4& normal, const glm::vec4& highlighted);
    void setItemSpacing(float pixels);
    void setCursorGlyph(char glyph);

private:
    std::vector<std::string> m_items;
    int       m_index     = 0;
    bool      m_confirmed = false;
    float     m_fontScale = 2.f;
    float     m_spacing   = 20.f;
    char      m_cursor    = '>';
    glm::vec4 m_colorNormal      = { 1.f, 1.f, 1.f, 1.f };
    glm::vec4 m_colorHighlighted = { 1.f, 1.f, 0.f, 1.f };
};

} // namespace Engine
