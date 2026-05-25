#pragma once
#include "AsteroidsConfig.h"
#include "AsteroidConfig.h"
#include <engine/facade/Collision.h>
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

    ~Asteroid();
    Asteroid(const Asteroid&)            = delete;
    Asteroid& operator=(const Asteroid&) = delete;

    bool  wasShot()    const { return m_wasShot; }
    float radius()     const { return AsteroidSizeConfigs::All[static_cast<int>(size)].cellCount * 8.f * SCALE; }
    int   scoreValue() const { return AsteroidSizeConfigs::All[static_cast<int>(size)].score; }

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
    int                    m_frameIndex     = 0;
    Engine::Collision::ColliderHandle m_colliderHandle = Engine::Collision::NULL_COLLIDER;
    bool                   m_wasShot        = false;

    Asteroid() = default;
    void registerCollider();
    void playDestructionSound()    const;
    void emitDestructionParticles() const;

};
