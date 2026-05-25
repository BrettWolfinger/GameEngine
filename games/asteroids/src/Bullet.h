#pragma once
#include "AsteroidsConfig.h"
#include <engine/facade/Collision.h>
#include <engine/renderer/Renderer2D.h>
#include <engine/renderer/SpriteSheet.h>
#include <engine/renderer/Texture.h>
#include <glm/glm.hpp>

class Bullet {
public:
    glm::vec2              pos;
    glm::vec2              vel;
    float                  lifetime;
    Engine::UVRect         uv  = { 0.f, 0.f, 1.f, 1.f };
    const Engine::Texture* tex = nullptr;

    static constexpr float SPEED    = 600.f;
    static constexpr float LIFETIME = 1.5f;
    static constexpr float SIZE     = 16.f * SCALE;

    Bullet(glm::vec2 pos, glm::vec2 vel, Engine::UVRect uv, const Engine::Texture* tex,
           uint32_t selfLayer = kBulletLayer, uint32_t targetLayer = kAsteroidLayer | kUfoLayer);
    ~Bullet();
    Bullet(const Bullet&)            = delete;
    Bullet& operator=(const Bullet&) = delete;

    bool isAlive() const { return lifetime > 0.f; }

    void update(float dt);
    void render(Engine::Renderer2D& renderer) const;

private:
    Engine::Collision::ColliderHandle m_colliderHandle = Engine::Collision::NULL_COLLIDER;
};
