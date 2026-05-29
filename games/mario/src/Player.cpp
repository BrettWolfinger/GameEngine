#include "Player.h"
#include "GameConstants.h"
#include <engine/core/Input.h>
#include <GLFW/glfw3.h>
#include <algorithm>

static constexpr float kGravity    = 1800.f;  // px/s²
static constexpr float kJumpVel    = -750.f;  // px/s upward
static constexpr float kJumpCutVel = -300.f;  // upward velocity cap on early release
static constexpr float kWalkSpeed  = 140.f;   // px/s max walk
static constexpr float kRunSpeed   = 230.f;   // px/s max run (shift)
static constexpr float kAccel      = 600.f;   // px/s² ground acceleration
static constexpr float kDecel      = 500.f;   // px/s² deceleration when no input
static constexpr float kSkidDecel  = 900.f;   // px/s² deceleration when pressing opposite direction

// Placeholder floor — row (SCREEN_ROWS - 2) in the tilemap; replaced by terrain collision later
static constexpr float kFloorTop  = static_cast<float>((SCREEN_ROWS - 2) * TILE * SCALE);

Player::Player(std::shared_ptr<Engine::SpriteSheet> sheet, float startX, float startY)
    : m_x(startX), m_y(startY), m_animator(sheet)
{
    m_animator.addClip("idle",   { {0},       0.2f });
    m_animator.addClip("crouch", { {1},       0.2f });
    m_animator.addClip("walk",   { {2, 3, 4}, 0.10f });
    m_animator.addClip("run",    { {2, 3, 4}, 0.06f });
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
    const bool right       = Engine::Input::isKeyDown(GLFW_KEY_RIGHT);
    const bool left        = Engine::Input::isKeyDown(GLFW_KEY_LEFT);
    const bool jumpPressed = Engine::Input::isKeyPressed(GLFW_KEY_SPACE);
    const bool jumpHeld    = Engine::Input::isKeyDown(GLFW_KEY_SPACE);

    m_crouching = Engine::Input::isKeyDown(GLFW_KEY_DOWN) && m_onGround;
    m_runHeld   = Engine::Input::isKeyDown(GLFW_KEY_LEFT_SHIFT);

    m_inputDir = 0;
    if (!m_crouching) {
        if (right) m_inputDir =  1;
        if (left)  m_inputDir = -1;
    }

    m_skidding = m_onGround && m_vx != 0.f && m_inputDir != 0 &&
                 ((m_inputDir > 0 && m_vx < 0.f) || (m_inputDir < 0 && m_vx > 0.f));

    if (m_skidding) {
        m_facingRight = m_vx > 0.f; // face the direction of travel, not the new input
    } else if (!m_crouching && m_inputDir != 0) {
        m_facingRight = m_inputDir > 0;
    }

    if (jumpPressed && m_onGround) {
        m_vy       = kJumpVel;
        m_onGround = false;
        m_jumpHeld = true;
    }
    if (m_jumpHeld && !jumpHeld) {
        m_jumpHeld = false;
        if (m_vy < kJumpCutVel)
            m_vy = kJumpCutVel;
    }
}

void Player::applyPhysics(float dt) {
    const float maxSpeed = m_runHeld ? kRunSpeed : kWalkSpeed;

    if (m_inputDir != 0) {
        if (m_skidding) {
            const float drag = kSkidDecel * dt;
            m_vx = m_vx > 0.f ? std::max(0.f, m_vx - drag)
                               : std::min(0.f, m_vx + drag);
        } else {
            m_vx += static_cast<float>(m_inputDir) * kAccel * dt;
            m_vx = std::clamp(m_vx, -maxSpeed, maxSpeed);
        }
    } else {
        const float drag = kDecel * dt;
        if (m_vx > 0.f)      m_vx = std::max(0.f, m_vx - drag);
        else if (m_vx < 0.f) m_vx = std::min(0.f, m_vx + drag);
    }

    if (!m_onGround)
        m_vy += kGravity * dt;

    m_x += m_vx * dt;
    m_y += m_vy * dt;

    if (m_x < 0.f) { m_x = 0.f; if (m_vx < 0.f) m_vx = 0.f; }

    if (m_y + MARIO_FRAME_H * SCALE >= kFloorTop) {
        m_y        = kFloorTop - MARIO_FRAME_H * SCALE;
        m_vy       = 0.f;
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
        case State::Walking:   clip = m_runHeld ? "run" : "walk"; break;
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
