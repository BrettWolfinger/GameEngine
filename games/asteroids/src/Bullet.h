#pragma once
#include <engine/renderer/Renderer2D.h>
#include <glm/glm.hpp>

struct Bullet {
    glm::vec2 pos;
    glm::vec2 vel;
    float     lifetime;

    static constexpr float SPEED    = 600.f;
    static constexpr float LIFETIME = 1.5f;
    static constexpr float SIZE     = 4.f;

    bool isAlive() const { return lifetime > 0.f; }

    void update(float dt);
    void render(Engine::Renderer2D& renderer) const;
};
