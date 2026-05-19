#include "Platform.h"

Platform::Platform(float startX, int row, PlatformType type, float speed, int direction)
    : m_x(startX), m_row(row), m_type(type), m_speed(speed), m_direction(direction)
{}

int Platform::tileWidth() const {
    return PLATFORM_TYPE_INFO[static_cast<int>(m_type)].tileWidth;
}

void Platform::update(float dt) {
    m_x += m_speed * static_cast<float>(m_direction) * dt;

    const float pw = static_cast<float>(tileWidth() * TILE);
    if (m_direction > 0 && m_x > W)
        m_x -= W + pw;
    else if (m_direction < 0 && m_x + pw < 0)
        m_x += W + pw;
}

void Platform::render(Engine::Renderer2D& renderer, const Engine::SpriteSheet& /*sheet*/) const {
    const auto& info = PLATFORM_TYPE_INFO[static_cast<int>(m_type)];
    renderer.drawRect(m_x, static_cast<float>(m_row * TILE),
                      static_cast<float>(info.tileWidth * TILE), static_cast<float>(TILE),
                      { info.r, info.g, info.b, 1.f });
}
