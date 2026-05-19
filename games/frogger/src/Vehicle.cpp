#include "Vehicle.h"
#include <algorithm>

Vehicle::Vehicle(float startX, int row, VehicleType type, float speed, int direction)
    : m_x(startX), m_row(row), m_type(type), m_speed(speed), m_direction(direction)
{}

int Vehicle::tileWidth() const {
    return VEHICLE_TYPE_INFO[static_cast<int>(m_type)].tileWidth;
}

void Vehicle::update(float dt) {
    m_x += m_speed * static_cast<float>(m_direction) * dt;

    const float vw = static_cast<float>(tileWidth() * TILE);
    if (m_direction > 0 && m_x > W)
        m_x -= W + vw;
    else if (m_direction < 0 && m_x + vw < 0)
        m_x += W + vw;
}

void Vehicle::render(Engine::Renderer2D& renderer, const Engine::SpriteSheet& sheet) const {
    const auto& info = VEHICLE_TYPE_INFO[static_cast<int>(m_type)];
    Engine::UVRect uvs = sheet.getSpanUVs(info.spriteFrame, info.tileWidth);
    // Flip horizontally for left-moving vehicles by swapping u0/u1
    if (m_direction < 0) std::swap(uvs.u0, uvs.u1);
    renderer.drawTexturedRect(m_x, static_cast<float>(m_row * TILE),
                              static_cast<float>(info.tileWidth * TILE), static_cast<float>(TILE),
                              sheet.texture(), uvs.u0, uvs.v0, uvs.u1, uvs.v1);
}
