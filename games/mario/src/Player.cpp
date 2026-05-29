#include "Player.h"
#include "GameConstants.h"
#include <engine/core/Input.h>
#include <GLFW/glfw3.h>
#include <algorithm>



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

float Player::hitboxX() const { return m_x + static_cast<float>(MARIO_HITBOX_OFFSET_X * SCALE); }
float Player::hitboxY() const { return m_y + static_cast<float>(MARIO_HITBOX_OFFSET_Y * SCALE); }
float Player::hitboxW() const { return static_cast<float>(MARIO_HITBOX_W * SCALE); }
float Player::hitboxH() const { return static_cast<float>(MARIO_HITBOX_H * SCALE); }

void Player::onStompGoomba() {
    m_vy       = STOMP_BOUNCE_VEL;
    m_onGround = false;
}

void Player::onHitByEnemy() {
    m_dead = true;
}

void Player::respawn(float startX, float startY) {
    m_x        = startX;
    m_y        = startY;
    m_vx       = 0.f;
    m_vy       = 0.f;
    m_onGround = false;
    m_dead     = false;
}

void Player::update(float dt, const MarioConfig& cfg, const Engine::Tilemap::Collider& collider) {
    if (m_dead) return;
    handleInput(cfg);
    applyPhysics(dt, cfg, collider);
    updateAnimation();
    m_animator.update(dt);
}

void Player::handleInput(const MarioConfig& cfg) {
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
        m_facingRight = m_vx > 0.f;
    } else if (!m_crouching && m_inputDir != 0) {
        m_facingRight = m_inputDir > 0;
    }

    if (jumpPressed && m_onGround) {
        m_vy       = cfg.jumpVel;
        m_onGround = false;
        m_jumpHeld = true;
    }
    if (m_jumpHeld && !jumpHeld) {
        m_jumpHeld = false;
        if (m_vy < cfg.jumpCutVel)
            m_vy = cfg.jumpCutVel;
    }
}

void Player::applyPhysics(float dt, const MarioConfig& cfg, const Engine::Tilemap::Collider& collider) {
    const float maxSpeed = m_runHeld ? cfg.runSpeed : cfg.walkSpeed;

    if (m_inputDir != 0) {
        if (m_skidding) {
            const float drag = cfg.skidDecel * dt;
            m_vx = m_vx > 0.f ? std::max(0.f, m_vx - drag)
                               : std::min(0.f, m_vx + drag);
        } else {
            m_vx += static_cast<float>(m_inputDir) * cfg.accel * dt;
            m_vx = std::clamp(m_vx, -maxSpeed, maxSpeed);
        }
    } else {
        const float drag = cfg.decel * dt;
        if (m_vx > 0.f)      m_vx = std::max(0.f, m_vx - drag);
        else if (m_vx < 0.f) m_vx = std::min(0.f, m_vx + drag);
    }

    if (!m_onGround)
        m_vy += cfg.gravity * dt;

    resolveCollision(dt, collider);
}

void Player::resolveCollision(float dt, const Engine::Tilemap::Collider& collider) {
    const float hx = m_x + static_cast<float>(MARIO_HITBOX_OFFSET_X * SCALE);
    const float hy = m_y + static_cast<float>(MARIO_HITBOX_OFFSET_Y * SCALE);
    const float hw = static_cast<float>(MARIO_HITBOX_W * SCALE);
    const float hh = static_cast<float>(MARIO_HITBOX_H * SCALE);

    const auto hit = collider.sweep(hx, hy, hw, hh, m_vx * dt, m_vy * dt);

    m_x += hit.dx;
    m_y += hit.dy;

    // Left-edge clamp: keep hitbox left >= 0
    if (m_x + MARIO_HITBOX_OFFSET_X * SCALE < 0.f) {
        m_x = -static_cast<float>(MARIO_HITBOX_OFFSET_X * SCALE);
        if (m_vx < 0.f) m_vx = 0.f;
    }

    if (hit.hitX) m_vx = 0.f;
    if (hit.hitY) {
        m_onGround = m_vy > 0.f;
        m_vy = 0.f;
    } else {
        // Probe 1px below hitbox feet — detects standing on ground when vy is zero
        const float resolvedHx = hx + hit.dx;
        const float resolvedHy = hy + hit.dy;
        m_onGround = collider.isSolidAt(resolvedHx + 1.f,      resolvedHy + hh + 1.f) ||
                     collider.isSolidAt(resolvedHx + hw - 1.f, resolvedHy + hh + 1.f);
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
