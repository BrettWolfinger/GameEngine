#pragma once
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

private:
    static constexpr float ROTATE_SPEED      = 3.0f;
    static constexpr float THRUST_FORCE      = 250.f;
    static constexpr float MAX_SPEED         = 450.f;
    static constexpr float DRAG              = 0.98f;
    static constexpr float SHIP_RENDER_SIZE  = 48.f;

    glm::vec2 m_pos;
    float     m_angle;
    glm::vec2 m_vel;
    bool      m_thrusting;
    std::string m_currentClip;

    Engine::SpriteAnimator m_animator;
};
