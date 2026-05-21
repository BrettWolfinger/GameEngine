#include "Ship.h"
#include "AsteroidsConfig.h"
#include <engine/core/Input.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <cmath>
#include <algorithm>

Ship::Ship(std::shared_ptr<Engine::SpriteSheet> sheet)
    : m_pos(W * 0.5f, H * 0.5f)
    , m_angle(0.f)
    , m_vel(0.f, 0.f)
    , m_thrusting(false)
    , m_animator(std::move(sheet))
{
    Engine::AnimClip idleClip;
    idleClip.frames        = { 0 };
    idleClip.frameDuration = 0.1f;
    idleClip.mode          = Engine::PlayMode::Loop;
    idleClip.wFrames       = 2;
    idleClip.hFrames       = 2;

    Engine::AnimClip thrustClip;
    thrustClip.frames        = { 4, 8 };
    thrustClip.frameDuration = 0.08f;
    thrustClip.mode          = Engine::PlayMode::Loop;
    thrustClip.wFrames       = 2;
    thrustClip.hFrames       = 2;

    m_animator.addClip("idle",   std::move(idleClip));
    m_animator.addClip("thrust", std::move(thrustClip));

    m_currentClip = "idle";
    m_animator.setClip("idle");
}

void Ship::reset() {
    m_pos       = { W * 0.5f, H * 0.5f };
    m_angle     = 0.f;
    m_vel       = { 0.f, 0.f };
    m_thrusting  = false;
    m_fireTimer  = 0.f;
    m_flashTimer = 0.f;
    m_currentClip = "idle";
    m_animator.setClip("idle");
}

bool Ship::tryShoot() {
    if (m_fireTimer > 0.f || !Engine::Input::isKeyDown(GLFW_KEY_SPACE))
        return false;
    m_fireTimer  = FIRE_COOLDOWN;
    m_flashTimer = FLASH_DURATION;
    return true;
}

void Ship::update(float dt, int screenW, int screenH) {
    if (Engine::Input::isKeyDown(GLFW_KEY_LEFT) || Engine::Input::isKeyDown(GLFW_KEY_A))
        m_angle -= ROTATE_SPEED * dt;
    if (Engine::Input::isKeyDown(GLFW_KEY_RIGHT) || Engine::Input::isKeyDown(GLFW_KEY_D))
        m_angle += ROTATE_SPEED * dt;

    m_thrusting = Engine::Input::isKeyDown(GLFW_KEY_UP) || Engine::Input::isKeyDown(GLFW_KEY_W);

    if (m_thrusting) {
        glm::vec2 forward = { glm::sin(m_angle), -glm::cos(m_angle) };
        m_vel += forward * (THRUST_FORCE * dt);
        float speed = glm::length(m_vel);
        if (speed > MAX_SPEED)
            m_vel *= MAX_SPEED / speed;
    }

    if (m_fireTimer  > 0.f) m_fireTimer  -= dt;
    if (m_flashTimer > 0.f) m_flashTimer -= dt;

    float dragFactor = std::pow(DRAG, dt * 60.f);
    m_vel *= dragFactor;

    m_pos += m_vel * dt;

    if (m_pos.x < -16.f)              m_pos.x += screenW + 32.f;
    if (m_pos.x > screenW + 16.f)     m_pos.x -= screenW + 32.f;
    if (m_pos.y < -16.f)              m_pos.y += screenH + 32.f;
    if (m_pos.y > screenH + 16.f)     m_pos.y -= screenH + 32.f;

    const std::string& targetClip = m_thrusting ? "thrust" : "idle";
    if (targetClip != m_currentClip) {
        m_currentClip = targetClip;
        m_animator.setClip(m_currentClip);
    }

    m_animator.update(dt);
}

void Ship::render(Engine::Renderer2D& renderer) const {
    const float half = RENDER_SIZE * 0.5f;
    const Engine::Texture& tex = m_animator.sheet().texture();

    if (m_thrusting) {
        glm::vec2 backward = { -glm::sin(m_angle), glm::cos(m_angle) };
        const float tx = m_pos.x + backward.x * 16.f - half;
        const float ty = m_pos.y + backward.y * 16.f - half;
        const Engine::UVRect thrUVs = m_animator.currentFrameUVs();
        renderer.drawTexturedRect(tx, ty, RENDER_SIZE, RENDER_SIZE,
                                  tex, thrUVs.u0, thrUVs.v0, thrUVs.u1, thrUVs.v1,
                                  m_angle);
    }

    const Engine::UVRect shipUVs = m_animator.sheet().getFrameUVs(0, 2, 2);
    renderer.drawTexturedRect(m_pos.x - half, m_pos.y - half, RENDER_SIZE, RENDER_SIZE,
                              tex, shipUVs.u0, shipUVs.v0, shipUVs.u1, shipUVs.v1,
                              m_angle);

    if (m_flashTimer > 0.f) {
        static constexpr int FLASH_FRAMES[3] = { 68, 72, 76 };
        static constexpr float FLASH_SIZE = 16.f * SCALE;
        int frame = std::clamp((int)((1.f - m_flashTimer / FLASH_DURATION) * 3.f), 0, 2);
        glm::vec2 forward     = { glm::sin(m_angle), -glm::cos(m_angle) };
        glm::vec2 nose        = m_pos + forward * (RENDER_SIZE * 0.5f);
        // Content sits at the bottom of the 16x16 frame; shift center forward by half
        // the render size so the content (bottom edge) lands at the nose.
        glm::vec2 flashCenter = nose + forward * (FLASH_SIZE * 0.5f);
        const Engine::UVRect uv = m_animator.sheet().getFrameUVs(FLASH_FRAMES[frame]);
        renderer.drawTexturedRect(flashCenter.x - FLASH_SIZE * 0.5f, flashCenter.y - FLASH_SIZE * 0.5f,
                                  FLASH_SIZE, FLASH_SIZE,
                                  tex, uv.u0, uv.v0, uv.u1, uv.v1, m_angle);
    }
}
