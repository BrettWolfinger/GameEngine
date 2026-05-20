#include "Platform.h"

Platform::Platform(float startX, int row, PlatformType type, int tileWidth, float speed, int direction)
    : LaneObject(startX, row, tileWidth, speed, direction)
    , m_type(type)
{}

void Platform::render(Engine::Renderer2D& renderer, const Engine::SpriteSheet& sheet) const {
    const auto& info = PLATFORM_TYPE_INFO[static_cast<int>(m_type)];
    for (int i = 0; i < m_tileWidth; ++i) {
        int frame;
        if      (i == 0)             frame = info.frameFirst;
        else if (i == m_tileWidth-1) frame = info.frameLast;
        else                         frame = info.frameMid;

        const Engine::UVRect uvs = sheet.getFrameUVs(frame);
        renderer.drawTexturedRect(m_x + static_cast<float>(i * TILE),
                                  static_cast<float>(m_row * TILE),
                                  static_cast<float>(TILE), static_cast<float>(TILE),
                                  sheet.texture(),
                                  uvs.u0, uvs.v0, uvs.u1, uvs.v1);
    }
}
