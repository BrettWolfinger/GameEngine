#include "Frog.h"
#include "FroggerConfig.h"
#include <engine/core/Input.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/constants.hpp>
#include <algorithm>

Frog::Frog(std::shared_ptr<Engine::SpriteSheet> sheet)
    : m_animator(std::move(sheet))
{
    Engine::AnimClip idleClip;
    idleClip.frames        = { 0 };
    idleClip.frameDuration = 1.f;
    idleClip.mode          = Engine::PlayMode::Loop;

    Engine::AnimClip hopClip;
    hopClip.frames        = { 0, 1, 2, 3 };
    hopClip.frameDuration = HOP_FRAME_DT;
    hopClip.mode          = Engine::PlayMode::Loop;

    m_animator.addClip("idle", std::move(idleClip));
    m_animator.addClip("hop",  std::move(hopClip));
    m_animator.setClip("idle");
}

void Frog::reset() {
    m_col        = 6;
    m_row        = 13;
    m_angle      = 3.14159265f;
    m_hopping    = false;
    m_hopTimer   = 0.f;
    m_rideOffset = 0.f;
    m_animator.setClip("idle");
}

void Frog::teleport(int col, int row) {
    m_col        = col;
    m_row        = row;
    m_angle      = 3.14159265f;
    m_hopping    = false;
    m_hopTimer   = 0.f;
    m_rideOffset = 0.f;
    m_animator.setClip("idle");
}

void Frog::applyRide(float dx) {
    m_rideOffset += dx;
    while (m_rideOffset >= static_cast<float>(TILE)) {
        ++m_col;
        m_rideOffset -= static_cast<float>(TILE);
    }
    while (m_rideOffset <= -static_cast<float>(TILE)) {
        --m_col;
        m_rideOffset += static_cast<float>(TILE);
    }
}

void Frog::update(float dt) {
    if (m_hopping) {
        m_hopTimer -= dt;
        if (m_hopTimer <= 0.f) {
            m_hopping = false;
            m_animator.setClip("idle");
        }
    } else {
        int dc = 0, dr = 0;
        if (Engine::Input::isKeyPressed(GLFW_KEY_UP)    || Engine::Input::isKeyPressed(GLFW_KEY_W)) dr = -1;
        if (Engine::Input::isKeyPressed(GLFW_KEY_DOWN)  || Engine::Input::isKeyPressed(GLFW_KEY_S)) dr =  1;
        if (Engine::Input::isKeyPressed(GLFW_KEY_LEFT)  || Engine::Input::isKeyPressed(GLFW_KEY_A)) dc = -1;
        if (Engine::Input::isKeyPressed(GLFW_KEY_RIGHT) || Engine::Input::isKeyPressed(GLFW_KEY_D)) dc =  1;

        if (dc != 0 || dr != 0) {
            m_rideOffset = 0.f;
            m_col = std::clamp(m_col + dc, 0, COLS - 1);
            m_row = std::clamp(m_row + dr, 0, ROWS - 1);
            m_hopping  = true;
            m_hopTimer = HOP_DURATION;
            if      (dr < 0) m_angle = glm::radians(180.f);
            else if (dr > 0) m_angle = 0.f;
            else if (dc > 0) m_angle = glm::radians(-90.f);
            else             m_angle = glm::radians(90.f);
            m_animator.setClip("hop");
        }
    }

    m_animator.update(dt);
}

void Frog::render(Engine::Renderer2D& renderer) const {
    const float x = pixelX();
    const float y = static_cast<float>(m_row * TILE);
    const Engine::UVRect uvs = m_animator.currentFrameUVs();
    renderer.drawTexturedRect(x, y, TILE, TILE,
                              m_animator.sheet().texture(),
                              uvs.u0, uvs.v0, uvs.u1, uvs.v1,
                              m_angle);
}
