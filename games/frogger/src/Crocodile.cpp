#include "Crocodile.h"
#include "FroggerConfig.h"
#include <algorithm>

Crocodile::Crocodile(float startX, int row, float speed, int direction)
    : Platform(startX, row, TILE_WIDTH, speed, direction)
{}

void Crocodile::onUpdate(float dt) {
    m_animTimer += dt;
    if (m_animTimer >= FRAME_DURATION) {
        m_animTimer -= FRAME_DURATION;
        m_frameIdx = (m_frameIdx + 1) % FRAME_COUNT;
    }
}

void Crocodile::render(Engine::Renderer2D& renderer, const Engine::SpriteSheet& sheet) const {
    // Right-moving: tail → body → head. Left-moving: head → body → tail, each tile flipped.
    const bool flip = m_direction < 0;

    const int frames[TILE_WIDTH] = {
        flip ? HEAD_FRAMES[m_frameIdx] : TAIL_FRAMES[m_frameIdx],
        BODY_FRAMES[m_frameIdx],
        flip ? TAIL_FRAMES[m_frameIdx] : HEAD_FRAMES[m_frameIdx],
    };

    for (int i = 0; i < TILE_WIDTH; ++i) {
        Engine::UVRect uvs = sheet.getFrameUVs(frames[i]);
        if (flip) std::swap(uvs.u0, uvs.u1);
        renderer.drawTexturedRect(m_x + static_cast<float>(i * TILE),
                                  static_cast<float>(m_row * TILE),
                                  static_cast<float>(TILE), static_cast<float>(TILE),
                                  sheet.texture(),
                                  uvs.u0, uvs.v0, uvs.u1, uvs.v1);
    }
}
