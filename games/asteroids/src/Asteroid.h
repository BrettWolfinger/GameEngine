#pragma once
#include "AsteroidsConfig.h"
#include "AsteroidConfig.h"
#include <engine/core/Services.h>
#include <engine/physics/Collider.h>
#include <engine/renderer/Renderer2D.h>
#include <engine/renderer/SpriteSheet.h>
#include <glm/glm.hpp>
#include <memory>
#include <random>
#include <vector>

enum class AsteroidSize { Large, Medium, Small };

class Asteroid {
public:
    glm::vec2    pos;
    glm::vec2    vel;
    float        angle    = 0.f;
    float        rotSpeed = 0.f;
    AsteroidSize size     = AsteroidSize::Large;
    int          variant  = 0;
    int          frameIndex = 0;
    int          cellCount  = 4;

    ~Asteroid();
    Asteroid(const Asteroid&)            = delete;
    Asteroid& operator=(const Asteroid&) = delete;

    bool  wasShot()    const { return m_wasShot; }
    float radius()     const { return cellCount * 8.f * SCALE; }
    int   scoreValue() const;

    static constexpr int SCORE_LARGE  = 20;
    static constexpr int SCORE_MEDIUM = 50;
    static constexpr int SCORE_SMALL  = 100;

    // Returns the fragments this asteroid splits into when destroyed.
    // Empty for Small asteroids. Safe to call from onUpdate (not from a callback).
    std::vector<std::unique_ptr<Asteroid>> split() const;

    void update(float dt, int screenW, int screenH);
    void render(Engine::Renderer2D& renderer, const Engine::SpriteSheet& sheet) const;

    // Spawns a large asteroid at pos with randomised velocity and rotation.
    static std::unique_ptr<Asteroid> spawnLarge(glm::vec2 pos, std::mt19937& rng);

    static std::unique_ptr<Asteroid> makeLarge (                    glm::vec2 pos, glm::vec2 vel, float rotSpeed);
    static std::unique_ptr<Asteroid> makeMedium(int variant,        glm::vec2 pos, glm::vec2 vel, float rotSpeed);
    static std::unique_ptr<Asteroid> makeSmall (int group, int idx, glm::vec2 pos, glm::vec2 vel, float rotSpeed);

private:
    Engine::ColliderHandle m_colliderHandle = Engine::NULL_COLLIDER;
    bool                   m_wasShot        = false;

    Asteroid() = default;
    void registerCollider();
    void playDestructionSound()    const;
    void emitDestructionParticles() const;

    static constexpr int MEDIUM_FRAMES[4]    = { 200, 202, 232, 234 };
    static constexpr int SMALL_FRAMES[4][4]  = {
        { 204, 205, 220, 221 },
        { 206, 207, 222, 223 },
        { 236, 237, 252, 253 },
        { 238, 239, 254, 255 },
    };
};
