#pragma once
#include "AsteroidsConfig.h"
#include <engine/physics/Collider.h>
#include <engine/renderer/Renderer2D.h>
#include <engine/renderer/SpriteSheet.h>
#include <engine/renderer/Texture.h>
#include <glm/glm.hpp>

struct Bullet {
    glm::vec2              pos;
    glm::vec2              vel;
    float                  lifetime;
    Engine::ColliderHandle colliderHandle = Engine::NULL_COLLIDER;
    Engine::UVRect         uv  = { 0.f, 0.f, 1.f, 1.f };
    const Engine::Texture* tex = nullptr;

    static constexpr float SPEED    = 600.f;
    static constexpr float LIFETIME = 1.5f;
    static constexpr float SIZE     = 16.f * SCALE;

    bool isAlive() const { return lifetime > 0.f; }

    void update(float dt);
    void render(Engine::Renderer2D& renderer) const;
};
