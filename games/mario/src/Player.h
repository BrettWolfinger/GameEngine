#pragma once
#include <engine/renderer/Renderer2D.h>
#include <engine/renderer/SpriteAnimator.h>
#include <memory>
#include <string>

class Player {
public:
    Player(std::shared_ptr<Engine::SpriteSheet> sheet, float startX, float startY);

    void update(float dt);
    void render(Engine::Renderer2D& renderer, float cameraX, int layer) const;

    float x() const { return m_x; }
    float y() const { return m_y; }

private:
    enum class State { Idle, Walking, Skidding, Jumping, Crouching };

    void handleInput();
    void applyPhysics(float dt);
    void updateAnimation();

    float  m_x, m_y;
    float  m_vx       = 0.f;
    float  m_vy       = 0.f;
    bool   m_onGround    = true;
    bool   m_facingRight = true;
    bool   m_crouching   = false;
    bool   m_skidding    = false;
    bool   m_runHeld     = false;
    bool   m_jumpHeld    = false;
    int    m_inputDir    = 0;   // -1 left, 0 none, 1 right
    State  m_state       = State::Idle;

    Engine::SpriteAnimator m_animator;
    std::string            m_currentClip;
};
