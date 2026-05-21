#include "Vehicle.h"
#include <algorithm>

Vehicle::Vehicle(float startX, int row, VehicleType type, float speed, int direction)
    : LaneObject(startX, row, VEHICLE_TYPE_INFO[static_cast<int>(type)].tileWidth, speed, direction)
    , m_type(type)
{}

void Vehicle::render(Engine::Renderer2D& renderer, const Engine::SpriteSheet& sheet) const {
    const auto& info = VEHICLE_TYPE_INFO[static_cast<int>(m_type)];
    Engine::UVRect uvs = sheet.getFrameUVs(info.spriteFrame, info.tileWidth, 1);
    if (m_direction < 0) std::swap(uvs.u0, uvs.u1);
    renderer.drawTexturedRect(m_x, static_cast<float>(m_row * TILE),
                              static_cast<float>(m_tileWidth * TILE), static_cast<float>(TILE),
                              sheet.texture(), uvs.u0, uvs.v0, uvs.u1, uvs.v1);
}
