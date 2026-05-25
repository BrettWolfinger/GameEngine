#pragma once
#include "AsteroidsConfig.h"
#include "UfoConfig.h"
#include <engine/facade/Collision.h>
#include <engine/renderer/Renderer2D.h>
#include <engine/renderer/SpriteSheet.h>
#include <glm/glm.hpp>
#include <optional>
#include <random>

enum class UfoSize { Large, Small };

class UFO {
public:
    struct BulletSpawn { glm::vec2 pos; glm::vec2 direction; };

    UFO(UfoSize size, glm::vec2 pos, glm::vec2 vel, std::mt19937& rng);
    ~UFO();
    UFO(const UFO&)            = delete;
    UFO& operator=(const UFO&) = delete;

    void update(float dt, int screenW, int screenH, glm::vec2 shipPos);
    void render(Engine::Renderer2D& renderer, const Engine::SpriteSheet& sheet) const;

    std::optional<BulletSpawn> tryFire(glm::vec2 shipPos);

    glm::vec2 pos()          const { return m_pos; }
    bool      wasDestroyed() const { return m_wasDestroyed; }
    bool      hasExited(int screenW, int screenH) const;
    UfoSize   ufoSize()      const { return m_size; }
    int       scoreValue()   const { return UfoConfigs::All[static_cast<int>(m_size)].score; }

private:
    float renderSize()      const { return UfoConfigs::All[static_cast<int>(m_size)].renderSize; }
    float collisionRadius() const { return renderSize() * 0.45f; }
    void  emitDestructionParticles() const;

    UfoSize       m_size;
    glm::vec2     m_pos;
    glm::vec2     m_vel;
    float         m_zigzagTimer  = UfoConfigs::ZIGZAG_INTERVAL;
    float         m_fireTimer    = 1.0f;
    float         m_beepTimer    = 0.f;
    bool          m_beepHigh     = false;
    bool          m_wasDestroyed = false;
    std::mt19937& m_rng;

    Engine::ColliderHandle m_colliderHandle = Engine::NULL_COLLIDER;
};
