#pragma once
#include "AsteroidsConfig.h"
#include "ShipConfig.h"
#include <engine/physics/Collider.h>
#include <engine/renderer/SpriteAnimator.h>
#include <engine/renderer/Renderer2D.h>
#include <glm/glm.hpp>
#include <memory>
#include <optional>
#include <string>

class Ship {
public:
    struct BulletSpawn { glm::vec2 pos; glm::vec2 direction; };

    explicit Ship(std::shared_ptr<Engine::SpriteSheet> sheet, const ShipConfig& config = ShipConfigs::All[0]);
    ~Ship();
    Ship(const Ship&)            = delete;
    Ship& operator=(const Ship&) = delete;

    void update(float dt, int screenW, int screenH);
    void render(Engine::Renderer2D& renderer) const;
    void reset();

    glm::vec2 pos()          const { return m_pos; }
    float     angle()        const { return m_angle; }
    bool      wasHit()       const { return m_wasHit; }
    bool      isInvincible() const { return m_invincibleTimer > 0.f; }
    void      clearHit()           { m_wasHit = false; }
    int       frameIndex()   const { return m_config.shipFrame; }
    int       frameCells()   const { return m_frameCells; }

    std::optional<BulletSpawn> tryShoot();

    static constexpr float RENDER_SIZE        = 32.f * SCALE;
    static constexpr float COLLISION_RADIUS   = RENDER_SIZE * 0.3f;
    static constexpr float INVINCIBLE_DURATION = 2.f;

private:
    static constexpr float FLASH_DURATION = 0.15f;

    ShipConfig  m_config;
    glm::vec2   m_pos;
    float       m_angle;
    glm::vec2   m_vel;
    bool        m_thrusting;
    float       m_fireTimer        = 0.f;
    float       m_flashTimer       = 0.f;
    float       m_invincibleTimer  = 0.f;
    std::string m_currentClip;

    int                     m_frameCells     = 2;
    Engine::SpriteAnimator  m_animator;
    Engine::ColliderHandle  m_colliderHandle = Engine::NULL_COLLIDER;
    bool                    m_wasHit         = false;
};
