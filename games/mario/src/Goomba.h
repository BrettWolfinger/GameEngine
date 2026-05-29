#pragma once
#include "GameConstants.h"
#include "GoombaConfig.h"
#include <engine/renderer/Renderer2D.h>
#include <engine/renderer/SpriteAnimator.h>
#include <engine/renderer/SpriteSheet.h>
#include <engine/tilemap/TilemapCollider.h>
#include <memory>

class Goomba {
public:
    Goomba(std::shared_ptr<Engine::SpriteSheet> sheet, float x, float y);

    void update(float dt, float gravity, const GoombaConfig& cfg, const Engine::Tilemap::Collider& collider);
    void render(Engine::Renderer2D& renderer, float cameraX, int layer) const;

    void stomp();
    bool isDead() const { return m_dead; }

    float hitboxX() const { return m_x; }
    float hitboxY() const { return m_y; }
    float hitboxW() const { return static_cast<float>(GOOMBA_FRAME_W * SCALE); }
    float hitboxH() const { return static_cast<float>(GOOMBA_FRAME_H * SCALE); }

private:
    void resolveCollision(float dt, const GoombaConfig& cfg, const Engine::Tilemap::Collider& collider);

    float m_x, m_y;
    int   m_dir  = -1;  // -1 = left, 1 = right
    float m_vy   = 0.f;
    bool  m_dead = false;

    Engine::SpriteAnimator m_animator;
};
