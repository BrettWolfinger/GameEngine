#include "Turtle.h"
#include "FroggerConfig.h"

Turtle::Turtle(float startX, int row, int tileWidth, float speed, int direction)
    : Platform(startX, row, tileWidth, speed, direction)
{}

void Turtle::render(Engine::Renderer2D& renderer, const Engine::SpriteSheet& sheet) const {
    const Engine::UVRect uvs = sheet.getFrameUVs(FRAME);
    for (int i = 0; i < m_tileWidth; ++i) {
        renderer.drawTexturedRect(m_x + static_cast<float>(i * TILE),
                                  static_cast<float>(m_row * TILE),
                                  static_cast<float>(TILE), static_cast<float>(TILE),
                                  sheet.texture(),
                                  uvs.u0, uvs.v0, uvs.u1, uvs.v1);
    }
}
