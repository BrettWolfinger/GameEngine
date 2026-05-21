#pragma once
#include "AsteroidsConfig.h"
#include <engine/physics/Collider.h>
#include <engine/renderer/Renderer2D.h>
#include <engine/renderer/SpriteSheet.h>
#include <glm/glm.hpp>

enum class AsteroidSize { Large, Medium, Small, Dead };

struct Asteroid {
    glm::vec2    pos;
    glm::vec2    vel;
    float        angle     = 0.f;   // radians
    float        rotSpeed  = 0.f;   // radians/sec
    AsteroidSize           size           = AsteroidSize::Large;
    int                    variant        = 0;     // medium: 0-3; small: group 0-3
    int                    frameIndex     = 0;     // top-left frame in 16×16 grid
    int                    cellCount      = 4;     // cells per side
    Engine::ColliderHandle colliderHandle = Engine::NULL_COLLIDER;

    float radius() const { return cellCount * 8.f * SCALE; }

    void update(float dt, int screenW, int screenH);
    void render(Engine::Renderer2D& renderer, const Engine::SpriteSheet& sheet) const;

    static Asteroid makeLarge (                          glm::vec2 pos, glm::vec2 vel, float rotSpeed);
    static Asteroid makeMedium(int variant,              glm::vec2 pos, glm::vec2 vel, float rotSpeed);
    static Asteroid makeSmall (int group, int idx,       glm::vec2 pos, glm::vec2 vel, float rotSpeed);

private:
    // Frame table: medium variants (top-left frame index for each 2×2 sprite)
    static constexpr int MEDIUM_FRAMES[4] = { 200, 202, 232, 234 };

    // Frame table: small groups, 4 frames each (1×1 sprites)
    static constexpr int SMALL_FRAMES[4][4] = {
        { 204, 205, 220, 221 },
        { 206, 207, 222, 223 },
        { 236, 237, 252, 253 },
        { 238, 239, 254, 255 },
    };
};
