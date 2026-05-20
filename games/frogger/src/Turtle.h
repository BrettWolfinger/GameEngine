#pragma once
#include "Platform.h"
#include <engine/renderer/SpriteAnimator.h>
#include <memory>

enum class TurtleState { Surfaced, Diving, Submerged, Surfacing };

class Turtle : public Platform {
public:
    Turtle(float startX, int row, int tileWidth, float speed, int direction,
           std::shared_ptr<Engine::SpriteSheet> sheet, float phaseOffset = 0.f);

    void render(Engine::Renderer2D&, const Engine::SpriteSheet&) const override;
    bool isSafe() const override;

protected:
    void onUpdate(float dt) override;

private:
    static constexpr int   FRAME_SWIM_FIRST  = 16;    // row 2 col 0
    static constexpr int   FRAME_DIVE_FIRST  = 96;    // row 12 col 0
    static constexpr int   DIVE_FRAME_COUNT  = 8;
    static constexpr float FRAME_DURATION    = 0.12f;
    static constexpr float DIVE_DURATION     = DIVE_FRAME_COUNT * FRAME_DURATION;
    static constexpr float SURFACE_DURATION  = 5.0f;
    static constexpr float SUBMERGED_DURATION = 2.0f;

    TurtleState          m_state = TurtleState::Surfaced;
    float                m_timer = 0.f;
    Engine::SpriteAnimator m_animator;
};
