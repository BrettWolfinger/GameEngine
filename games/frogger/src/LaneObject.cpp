#include "LaneObject.h"

LaneObject::LaneObject(float startX, int row, int tileWidth, float speed, int direction)
    : m_x(startX), m_row(row), m_tileWidth(tileWidth), m_speed(speed), m_direction(direction)
{}

void LaneObject::update(float dt) {
    m_x += m_speed * static_cast<float>(m_direction) * dt;

    const float w = static_cast<float>(m_tileWidth * TILE);
    if (m_direction > 0 && m_x > W)
        m_x -= W + w;
    else if (m_direction < 0 && m_x + w < 0)
        m_x += W + w;

    onUpdate(dt);
}
