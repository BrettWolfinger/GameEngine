#pragma once
#include <engine/renderer/SpriteAnimator.h>
#include <engine/renderer/Renderer2D.h>
#include "FroggerConfig.h"
#include <memory>

class Frog {
public:
    explicit Frog(std::shared_ptr<Engine::SpriteSheet> sheet);

    void update(float dt);
    void render(Engine::Renderer2D& renderer) const;
    void reset();
    void teleport(int col, int row);
    void applyRide(float dx);

    int   col()    const { return m_col; }
    int   row()    const { return m_row; }
    float pixelX() const { return static_cast<float>(m_col * TILE) + m_rideOffset; }

private:
    static constexpr int   HOP_FRAMES   = 4;
    static constexpr float HOP_FRAME_DT = 0.05f;
    static constexpr float HOP_DURATION = HOP_FRAMES * HOP_FRAME_DT;

    int   m_col        = 6;
    int   m_row        = 13;
    float m_angle      = 0.f;
    bool  m_hopping    = false;
    float m_hopTimer   = 0.f;
    float m_rideOffset = 0.f;

    Engine::SpriteAnimator m_animator;
};
