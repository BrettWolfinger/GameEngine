#include "Platform.h"

Platform::Platform(float startX, int row, PlatformType type, int tileWidth, float speed, int direction)
    : m_x(startX), m_row(row), m_type(type), m_tileWidth(tileWidth), m_speed(speed), m_direction(direction)
{}

void Platform::update(float dt) {
    m_x += m_speed * static_cast<float>(m_direction) * dt;

    const float pw = static_cast<float>(m_tileWidth * TILE);
    if (m_direction > 0 && m_x > W)
        m_x -= W + pw;
    else if (m_direction < 0 && m_x + pw < 0)
        m_x += W + pw;
}

void Platform::render(Engine::Renderer2D& renderer, const Engine::SpriteSheet& sheet) const {
    const auto& info = PLATFORM_TYPE_INFO[static_cast<int>(m_type)];
    for (int i = 0; i < m_tileWidth; ++i) {
        int frame;
        if      (i == 0)              frame = info.frameFirst;
        else if (i == m_tileWidth-1)  frame = info.frameLast;
        else                          frame = info.frameMid;

        const Engine::UVRect uvs = sheet.getFrameUVs(frame);
        renderer.drawTexturedRect(m_x + static_cast<float>(i * TILE),
                                  static_cast<float>(m_row * TILE),
                                  static_cast<float>(TILE), static_cast<float>(TILE),
                                  sheet.texture(),
                                  uvs.u0, uvs.v0, uvs.u1, uvs.v1);
    }
}
