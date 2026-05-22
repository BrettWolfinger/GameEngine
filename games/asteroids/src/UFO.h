#pragma once
#include "AsteroidsConfig.h"
#include <engine/physics/Collider.h>
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

    static constexpr float LARGE_RENDER_SIZE = 32.f * SCALE;
    static constexpr float SMALL_RENDER_SIZE = 16.f * SCALE;
    static constexpr float LARGE_SPEED       = 80.f;
    static constexpr float SMALL_SPEED       = 120.f;
    static constexpr int   SCORE_LARGE       = 200;
    static constexpr int   SCORE_SMALL       = 1000;
    static constexpr int   SMALL_FRAME       = 12;
    static constexpr int   LARGE_FRAME       = 44;
    static constexpr int   SPRITE_CELLS      = 2;

private:
    static constexpr float ZIGZAG_INTERVAL    = 1.5f;
    static constexpr float ZIGZAG_MAX_YSPEED  = 60.f;
    static constexpr float LARGE_FIRE_RATE    = 2.0f;
    static constexpr float SMALL_FIRE_RATE    = 1.2f;
    static constexpr float BEEP_INTERVAL      = 0.45f;
    static constexpr float SMALL_AIM_VARIANCE = 0.15f;

    float renderSize()      const;
    float collisionRadius() const;
    void  emitDestructionParticles() const;

    UfoSize       m_size;
    glm::vec2     m_pos;
    glm::vec2     m_vel;
    float         m_zigzagTimer  = ZIGZAG_INTERVAL;
    float         m_fireTimer    = 1.0f;
    float         m_beepTimer    = 0.f;
    bool          m_beepHigh     = false;
    bool          m_wasDestroyed = false;
    std::mt19937& m_rng;

    Engine::ColliderHandle m_colliderHandle = Engine::NULL_COLLIDER;
};
