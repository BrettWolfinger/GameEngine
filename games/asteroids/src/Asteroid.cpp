#include "Asteroid.h"
#include "AsteroidsConfig.h"
#include <engine/core/Services.h>
#include <engine/physics/CollisionWorld.h>
#include <glm/gtc/constants.hpp>
#include <cmath>

constexpr int Asteroid::MEDIUM_FRAMES[4];
constexpr int Asteroid::SMALL_FRAMES[4][4];

// ---- lifecycle --------------------------------------------------------------

Asteroid::~Asteroid() {
    Engine::Services::collision().remove(m_colliderHandle);
}

void Asteroid::registerCollider() {
    m_colliderHandle = Engine::Services::collision().add(
        Engine::ColliderDesc::makeCircle(kAsteroidLayer, kBulletLayer, pos.x, pos.y, radius()),
        [this](Engine::ColliderHandle self, Engine::ColliderHandle) {
            m_wasShot = true;
            Engine::Services::collision().remove(self);
            m_colliderHandle = Engine::NULL_COLLIDER;
        });
}

// ---- factories --------------------------------------------------------------

std::unique_ptr<Asteroid> Asteroid::makeLarge(glm::vec2 pos, glm::vec2 vel, float rotSpeed) {
    auto a = std::unique_ptr<Asteroid>(new Asteroid());
    a->pos        = pos;
    a->vel        = vel;
    a->rotSpeed   = rotSpeed;
    a->size       = AsteroidSize::Large;
    a->frameIndex = 196;
    a->cellCount  = 4;
    a->registerCollider();
    return a;
}

std::unique_ptr<Asteroid> Asteroid::makeMedium(int variant, glm::vec2 pos, glm::vec2 vel, float rotSpeed) {
    auto a = std::unique_ptr<Asteroid>(new Asteroid());
    a->pos        = pos;
    a->vel        = vel;
    a->rotSpeed   = rotSpeed;
    a->size       = AsteroidSize::Medium;
    a->variant    = variant & 3;
    a->frameIndex = MEDIUM_FRAMES[a->variant];
    a->cellCount  = 2;
    a->registerCollider();
    return a;
}

std::unique_ptr<Asteroid> Asteroid::makeSmall(int group, int idx, glm::vec2 pos, glm::vec2 vel, float rotSpeed) {
    auto a = std::unique_ptr<Asteroid>(new Asteroid());
    a->pos        = pos;
    a->vel        = vel;
    a->rotSpeed   = rotSpeed;
    a->size       = AsteroidSize::Small;
    a->variant    = group & 3;
    a->frameIndex = SMALL_FRAMES[a->variant][idx & 3];
    a->cellCount  = 1;
    a->registerCollider();
    return a;
}

// ---- split ------------------------------------------------------------------

std::vector<std::unique_ptr<Asteroid>> Asteroid::split() const {
    std::vector<std::unique_ptr<Asteroid>> fragments;
    if (size == AsteroidSize::Small) return fragments;

    const float baseAngle = std::atan2(vel.y, vel.x);
    const float speed     = glm::length(vel) * 2.f;

    if (size == AsteroidSize::Large) {
        for (int i = 0; i < 4; ++i) {
            const float ang = baseAngle + glm::half_pi<float>() * i;
            fragments.push_back(makeMedium(i, pos,
                { std::cos(ang) * speed, std::sin(ang) * speed },
                1.0f * (i % 2 == 0 ? 1.f : -1.f)));
        }
    } else if (size == AsteroidSize::Medium) {
        for (int i = 0; i < 4; ++i) {
            const float ang = baseAngle + glm::half_pi<float>() * i;
            fragments.push_back(makeSmall(variant, i, pos,
                { std::cos(ang) * speed, std::sin(ang) * speed },
                1.5f * (i % 2 == 0 ? 1.f : -1.f)));
        }
    }

    return fragments;
}

// ---- update -----------------------------------------------------------------

void Asteroid::update(float dt, int screenW, int screenH) {
    pos   += vel * dt;
    angle += rotSpeed * dt;

    if (pos.x < -16.f)           pos.x += screenW + 32.f;
    if (pos.x > screenW + 16.f)  pos.x -= screenW + 32.f;
    if (pos.y < -16.f)           pos.y += screenH + 32.f;
    if (pos.y > screenH + 16.f)  pos.y -= screenH + 32.f;

    if (m_colliderHandle != Engine::NULL_COLLIDER)
        Engine::Services::collision().updateCircle(m_colliderHandle, pos.x, pos.y, radius());
}

// ---- render -----------------------------------------------------------------

void Asteroid::render(Engine::Renderer2D& renderer, const Engine::SpriteSheet& sheet) const {
    const float renderSize = cellCount * 16.f * SCALE;
    const float half       = renderSize * 0.5f;

    const Engine::UVRect uv = sheet.getFrameUVs(frameIndex, cellCount, cellCount);
    renderer.drawTexturedRect(pos.x - half, pos.y - half, renderSize, renderSize,
                              sheet.texture(),
                              uv.u0, uv.v0, uv.u1, uv.v1,
                              angle);
}
