#include "Player.h"
#include "GameConstants.h"
#include <engine/core/Input.h>
#include <GLFW/glfw3.h>

static constexpr float kGravity     = 1800.f;  // px/s²
static constexpr float kJumpVel     = -750.f;  // px/s, upward
static constexpr float kWalkSpeed   = 140.f;   // px/s

// Placeholder floor — row (SCREEN_ROWS - 2) in the tilemap; replaced by terrain collision later
static constexpr float kFloorTop  = static_cast<float>((SCREEN_ROWS - 2) * TILE * SCALE);

Player::Player(std::shared_ptr<Engine::SpriteSheet> sheet, float startX, float startY)
    : m_x(startX), m_y(startY), m_animator(sheet)
{
    m_animator.addClip("idle",   { {0},       0.2f });
    m_animator.addClip("crouch", { {1},       0.2f });
    m_animator.addClip("walk",   { {2, 3, 4}, 0.1f });
    m_animator.addClip("skid",   { {5},       0.2f });
    m_animator.addClip("jump",   { {6},       0.2f });
    m_animator.setClip("idle");
    m_currentClip = "idle";
}

void Player::update(float dt) {
    handleInput();
    applyPhysics(dt);
    updateAnimation();
    m_animator.update(dt);
}

void Player::handleInput() {
    const bool right = Engine::Input::isKeyDown(GLFW_KEY_RIGHT);
    const bool left  = Engine::Input::isKeyDown(GLFW_KEY_LEFT);
    m_crouching = Engine::Input::isKeyDown(GLFW_KEY_DOWN) && m_onGround;

    float newVx = 0.f;
    if (!m_crouching) {
        if (right)     newVx =  kWalkSpeed;
        else if (left) newVx = -kWalkSpeed;
    }

    // Skidding: reversing direction while on the ground
    m_skidding = m_onGround && ((m_vx > 0.f && newVx < 0.f) || (m_vx < 0.f && newVx > 0.f));

    // Facing direction holds while skidding (Mario faces the way he was going)
    if (!m_skidding && !m_crouching) {
        if (right)     m_facingRight = true;
        else if (left) m_facingRight = false;
    }

    m_vx = newVx;

    if (Engine::Input::isKeyPressed(GLFW_KEY_SPACE) && m_onGround) {
        m_vy = kJumpVel;
        m_onGround = false;
    }
}

void Player::applyPhysics(float dt) {
    if (!m_onGround)
        m_vy += kGravity * dt;

    m_x += m_vx * dt;
    m_y += m_vy * dt;

    if (m_y + MARIO_FRAME_H * SCALE >= kFloorTop) {
        m_y = kFloorTop - MARIO_FRAME_H * SCALE;
        m_vy = 0.f;
        m_onGround = true;
    }
}

void Player::updateAnimation() {
    if (!m_onGround)       m_state = State::Jumping;
    else if (m_crouching)  m_state = State::Crouching;
    else if (m_skidding)   m_state = State::Skidding;
    else if (m_vx != 0.f)  m_state = State::Walking;
    else                   m_state = State::Idle;

    const char* clip = "idle";
    switch (m_state) {
        case State::Walking:   clip = "walk";   break;
        case State::Skidding:  clip = "skid";   break;
        case State::Jumping:   clip = "jump";   break;
        case State::Crouching: clip = "crouch"; break;
        default: break;
    }

    if (m_currentClip != clip) {
        m_currentClip = clip;
        m_animator.setClip(clip);
    }
}

void Player::render(Engine::Renderer2D& renderer, float cameraX, int layer) const {
    const Engine::UVRect uv = m_animator.currentFrameUVs();
    const float w = static_cast<float>(MARIO_FRAME_W * SCALE);
    const float h = static_cast<float>(MARIO_FRAME_H * SCALE);

    // Flip UVs horizontally when facing left
    const float u0 = m_facingRight ? uv.u0 : uv.u1;
    const float u1 = m_facingRight ? uv.u1 : uv.u0;

    renderer.drawTexturedRect(m_x - cameraX, m_y, w, h,
                              m_animator.sheet().texture(),
                              u0, uv.v0, u1, uv.v1,
                              0.f, { 1.f, 1.f, 1.f, 1.f }, layer);
}
