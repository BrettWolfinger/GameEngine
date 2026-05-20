#include "Crocodile.h"
#include "FroggerConfig.h"
#include <algorithm>

Crocodile::Crocodile(float startX, int row, float speed, int direction, float mouthPhase)
    : Platform(startX, row, TILE_WIDTH, speed, direction)
    , m_mouthTimer(mouthPhase)
{}

bool Crocodile::isSafe(float frogPx) const {
    if (m_mouthState == MouthState::Closed) return true;
    const float headX = (m_direction > 0) ? m_x + 2.f * TILE : m_x;
    return !(frogPx < headX + TILE && frogPx + TILE > headX);
}

void Crocodile::onUpdate(float dt) {
    // Swim animation — all tiles
    m_animTimer += dt;
    if (m_animTimer >= FRAME_DURATION) {
        m_animTimer -= FRAME_DURATION;
        m_frameIdx = (m_frameIdx + 1) % FRAME_COUNT;
    }

    // Mouth state machine — head tile only
    m_mouthTimer += dt;
    switch (m_mouthState) {
        case MouthState::Closed:
            if (m_mouthTimer >= CLOSED_DURATION) {
                m_mouthState = MouthState::Opening;
                m_mouthTimer = 0.f;
            }
            break;
        case MouthState::Opening:
            if (m_mouthTimer >= MOUTH_OPEN_COUNT * MOUTH_FRAME_DT) {
                m_mouthState = MouthState::Open;
                m_mouthTimer = 0.f;
            }
            break;
        case MouthState::Open:
            if (m_mouthTimer >= OPEN_DURATION) {
                m_mouthState = MouthState::Closing;
                m_mouthTimer = 0.f;
            }
            break;
        case MouthState::Closing:
            if (m_mouthTimer >= MOUTH_OPEN_COUNT * MOUTH_FRAME_DT) {
                m_mouthState = MouthState::Closed;
                m_mouthTimer = 0.f;
            }
            break;
    }
}

void Crocodile::render(Engine::Renderer2D& renderer, const Engine::SpriteSheet& sheet) const {
    const bool flip = m_direction < 0;

    // Determine current head frame based on mouth state
    int headFrame;
    switch (m_mouthState) {
        case MouthState::Closed:
            headFrame = HEAD_FRAMES[m_frameIdx];
            break;
        case MouthState::Opening: {
            int mf = std::min(static_cast<int>(m_mouthTimer / MOUTH_FRAME_DT), MOUTH_OPEN_COUNT - 1);
            headFrame = MOUTH_OPEN_FRAMES[mf];
            break;
        }
        case MouthState::Open: {
            int mf = static_cast<int>(m_mouthTimer / MOUTH_FRAME_DT) % MOUTH_SWIM_COUNT;
            headFrame = MOUTH_SWIM_FRAMES[mf];
            break;
        }
        case MouthState::Closing: {
            int mf = MOUTH_OPEN_COUNT - 1 - std::min(static_cast<int>(m_mouthTimer / MOUTH_FRAME_DT), MOUTH_OPEN_COUNT - 1);
            headFrame = MOUTH_OPEN_FRAMES[mf];
            break;
        }
    }

    const int frames[TILE_WIDTH] = {
        flip ? headFrame             : TAIL_FRAMES[m_frameIdx],
        BODY_FRAMES[m_frameIdx],
        flip ? TAIL_FRAMES[m_frameIdx] : headFrame,
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
