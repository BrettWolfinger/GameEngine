#pragma once
#include "Contactable.h"
#include "GameConstants.h"
#include "MarioConfig.h"
#include <engine/facade/Collision.h>
#include <engine/renderer/Renderer2D.h>
#include <engine/renderer/SpriteAnimator.h>
#include <engine/tilemap/TilemapCollider.h>
#include <memory>
#include <string>

class Player : public IContactable {
public:
    Player(std::shared_ptr<Engine::SpriteSheet> sheet, float startX, float startY);
    ~Player();

    void registerColliders();
    void deregisterColliders();

    void update(float dt, const MarioConfig& cfg, const Engine::Tilemap::Collider& collider);
    void render(Engine::Renderer2D& renderer, float cameraX, int layer) const;

    float x() const { return m_x; }
    float y() const { return m_y; }
    float vy() const { return m_vy; }

    float hitboxX() const;
    float hitboxY() const;
    float hitboxW() const;
    float hitboxH() const;

    void applyContactEffect(const ContactEffect& effect) override;

    bool isDead() const { return m_dead; }
    void respawn(float startX, float startY);

private:
    enum class State { Idle, Walking, Skidding, Jumping, Crouching };

    void handleInput(const MarioConfig& cfg);
    void applyPhysics(float dt, const MarioConfig& cfg, const Engine::Tilemap::Collider& collider);
    void resolveCollision(float dt, const Engine::Tilemap::Collider& collider);
    void updateAnimation();

    float  m_x, m_y;
    float  m_vx          = 0.f;
    float  m_vy          = 0.f;
    bool   m_onGround    = true;
    bool   m_facingRight = true;
    bool   m_crouching   = false;
    bool   m_skidding    = false;
    bool   m_runHeld     = false;
    bool   m_jumpHeld    = false;
    int    m_inputDir    = 0;
    State  m_state       = State::Idle;
    bool   m_dead        = false;

    Engine::Collision::ColliderHandle m_bodyHandle  = Engine::Collision::NULL_COLLIDER;
    Engine::Collision::ColliderHandle m_stompHandle = Engine::Collision::NULL_COLLIDER;

    Engine::SpriteAnimator m_animator;
    std::string            m_currentClip;
};
