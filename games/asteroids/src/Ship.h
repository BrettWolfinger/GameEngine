#pragma once
#include "AsteroidsConfig.h"
#include <engine/renderer/SpriteAnimator.h>
#include <engine/renderer/Renderer2D.h>
#include <glm/glm.hpp>
#include <memory>
#include <string>

class Ship {
public:
    explicit Ship(std::shared_ptr<Engine::SpriteSheet> sheet);

    void update(float dt, int screenW, int screenH);
    void render(Engine::Renderer2D& renderer) const;
    void reset();

    glm::vec2 pos()      const { return m_pos; }
    float     angle()    const { return m_angle; }
    bool      tryShoot();

    static constexpr float RENDER_SIZE = 32.f * SCALE;

private:
    static constexpr float ROTATE_SPEED   = 3.0f;
    static constexpr float THRUST_FORCE   = 250.f;
    static constexpr float MAX_SPEED      = 450.f;
    static constexpr float DRAG           = 0.98f;
    static constexpr float FIRE_COOLDOWN  = 0.25f;
    static constexpr float FLASH_DURATION = 0.15f;

    glm::vec2 m_pos;
    float     m_angle;
    glm::vec2 m_vel;
    bool      m_thrusting;
    float     m_fireTimer  = 0.f;
    float     m_flashTimer = 0.f;
    std::string m_currentClip;

    Engine::SpriteAnimator m_animator;
};
