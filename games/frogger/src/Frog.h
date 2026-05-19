#pragma once
#include <engine/renderer/SpriteAnimator.h>
#include <engine/renderer/Renderer2D.h>
#include <memory>

class Frog {
public:
    explicit Frog(std::shared_ptr<Engine::SpriteSheet> sheet);

    void update(float dt);
    void render(Engine::Renderer2D& renderer) const;
    void reset();

    int col() const { return m_col; }
    int row() const { return m_row; }

private:
    static constexpr int   HOP_FRAMES   = 4;
    static constexpr float HOP_FRAME_DT = 0.1f;
    static constexpr float HOP_DURATION = HOP_FRAMES * HOP_FRAME_DT;

    int   m_col      = 6;
    int   m_row      = 13;
    float m_angle    = 3.14159265f;
    bool  m_hopping  = false;
    float m_hopTimer = 0.f;

    Engine::SpriteAnimator m_animator;
};
