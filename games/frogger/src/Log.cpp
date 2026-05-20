#include "Log.h"
#include "FroggerConfig.h"

Log::Log(float startX, int row, int tileWidth, float speed, int direction)
    : Platform(startX, row, tileWidth, speed, direction)
{}

void Log::render(Engine::Renderer2D& renderer, const Engine::SpriteSheet& sheet) const {
    for (int i = 0; i < m_tileWidth; ++i) {
        int frame;
        if      (i == 0)             frame = FRAME_FIRST;
        else if (i == m_tileWidth-1) frame = FRAME_LAST;
        else                         frame = FRAME_MID;

        const Engine::UVRect uvs = sheet.getFrameUVs(frame);
        renderer.drawTexturedRect(m_x + static_cast<float>(i * TILE),
                                  static_cast<float>(m_row * TILE),
                                  static_cast<float>(TILE), static_cast<float>(TILE),
                                  sheet.texture(),
                                  uvs.u0, uvs.v0, uvs.u1, uvs.v1);
    }
}
