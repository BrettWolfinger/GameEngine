#include "Turtle.h"
#include "FroggerConfig.h"

Turtle::Turtle(float startX, int row, int tileWidth, float speed, int direction,
               std::shared_ptr<Engine::SpriteSheet> sheet, float phaseOffset)
    : Platform(startX, row, tileWidth, speed, direction)
    , m_timer(phaseOffset)
    , m_animator(std::move(sheet))
{
    Engine::AnimClip surfaced;
    surfaced.frames        = { FRAME_SWIM_FIRST,     // row 2 col 0
                                FRAME_SWIM_FIRST + 1, // row 2 col 1
                                FRAME_SWIM_FIRST + 2, // row 2 col 2
                                FRAME_SWIM_FIRST + 3, // row 2 col 3
                                FRAME_SWIM_FIRST + 4  // row 2 col 4
                              };
    surfaced.frameDuration = 0.15f;
    surfaced.mode          = Engine::PlayMode::Loop;

    Engine::AnimClip diving;
    diving.frames        = { 96, 97, 98, 99, 100, 101, 102, 103 };
    diving.frameDuration = FRAME_DURATION;
    diving.mode          = Engine::PlayMode::OneShot;

    Engine::AnimClip submerged;
    submerged.frames        = { 103 };
    submerged.frameDuration = 1.f;
    submerged.mode          = Engine::PlayMode::Loop;

    Engine::AnimClip surfacing;
    surfacing.frames        = { 103, 102, 101, 100, 99, 98, 97, 96 };
    surfacing.frameDuration = FRAME_DURATION;
    surfacing.mode          = Engine::PlayMode::OneShot;

    m_animator.addClip("surfaced",  std::move(surfaced));
    m_animator.addClip("diving",    std::move(diving));
    m_animator.addClip("submerged", std::move(submerged));
    m_animator.addClip("surfacing", std::move(surfacing));
    m_animator.setClip("surfaced");
}

bool Turtle::isSafe(float /*frogPx*/) const {
    return m_state == TurtleState::Surfaced || m_state == TurtleState::Diving;
}

void Turtle::onUpdate(float dt) {
    m_timer += dt;
    m_animator.update(dt);

    switch (m_state) {
        case TurtleState::Surfaced:
            if (m_timer >= SURFACE_DURATION) {
                m_state = TurtleState::Diving;
                m_timer = 0.f;
                m_animator.setClip("diving");
            }
            break;

        case TurtleState::Diving:
            if (m_timer >= DIVE_DURATION) {
                m_state = TurtleState::Submerged;
                m_timer = 0.f;
                m_animator.setClip("submerged");
            }
            break;

        case TurtleState::Submerged:
            if (m_timer >= SUBMERGED_DURATION) {
                m_state = TurtleState::Surfacing;
                m_timer = 0.f;
                m_animator.setClip("surfacing");
            }
            break;

        case TurtleState::Surfacing:
            if (m_timer >= DIVE_DURATION) {
                m_state = TurtleState::Surfaced;
                m_timer = 0.f;
                m_animator.setClip("surfaced");
            }
            break;
    }
}

void Turtle::render(Engine::Renderer2D& renderer, const Engine::SpriteSheet& /*sheet*/) const {
    if (m_state == TurtleState::Submerged) return;

    const Engine::UVRect uvs = m_animator.currentFrameUVs();
    for (int i = 0; i < m_tileWidth; ++i) {
        renderer.drawTexturedRect(m_x + static_cast<float>(i * TILE),
                                  static_cast<float>(m_row * TILE),
                                  static_cast<float>(TILE), static_cast<float>(TILE),
                                  m_animator.sheet().texture(),
                                  uvs.u0, uvs.v0, uvs.u1, uvs.v1);
    }
}
