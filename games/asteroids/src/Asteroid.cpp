#include "Asteroid.h"
#include "AsteroidsConfig.h"
#include <engine/audio/AudioManager.h>
#include <engine/core/Services.h>
#include <engine/particles/ParticleSystem.h>
#include <engine/physics/CollisionWorld.h>
#include <glm/gtc/constants.hpp>
#include <cmath>

// ---- lifecycle --------------------------------------------------------------

Asteroid::~Asteroid() {
    Engine::Services::collision().remove(m_colliderHandle);
}

void Asteroid::registerCollider() {
    m_colliderHandle = Engine::Services::collision().add(
        Engine::ColliderDesc::makeCircle(kAsteroidLayer, kBulletLayer, pos.x, pos.y, radius()),
        [this](Engine::ColliderHandle self, Engine::ColliderHandle) {
            m_wasShot = true;
            playDestructionSound();
            emitDestructionParticles();
            Engine::Services::collision().remove(self);
            m_colliderHandle = Engine::NULL_COLLIDER;
        });
}

// ---- spawn ------------------------------------------------------------------

std::unique_ptr<Asteroid> Asteroid::spawnLarge(glm::vec2 pos, std::mt19937& rng) {
    const float velAngle = std::uniform_real_distribution<float>(0.f, 6.2831853f)(rng);
    const float speed    = std::uniform_real_distribution<float>(40.f, 80.f)(rng);
    const float rotMag   = std::uniform_real_distribution<float>(0.5f, 1.5f)(rng);
    const float rotSign  = std::uniform_int_distribution<int>(0, 1)(rng) ? 1.f : -1.f;
    return makeLarge(pos,
                     { std::cos(velAngle) * speed, std::sin(velAngle) * speed },
                     rotMag * rotSign);
}

// ---- factories --------------------------------------------------------------

std::unique_ptr<Asteroid> Asteroid::makeLarge(glm::vec2 pos, glm::vec2 vel, float rotSpeed) {
    const auto& cfg = AsteroidSizeConfigs::All[static_cast<int>(AsteroidSize::Large)];
    auto a = std::unique_ptr<Asteroid>(new Asteroid());
    a->pos        = pos;
    a->vel        = vel;
    a->rotSpeed   = rotSpeed;
    a->size         = AsteroidSize::Large;
    a->m_frameIndex = cfg.frames[0][0];
    a->registerCollider();
    return a;
}

std::unique_ptr<Asteroid> Asteroid::makeMedium(int variant, glm::vec2 pos, glm::vec2 vel, float rotSpeed) {
    const auto& cfg = AsteroidSizeConfigs::All[static_cast<int>(AsteroidSize::Medium)];
    auto a = std::unique_ptr<Asteroid>(new Asteroid());
    a->pos        = pos;
    a->vel        = vel;
    a->rotSpeed   = rotSpeed;
    a->size         = AsteroidSize::Medium;
    a->variant      = variant & 3;
    a->m_frameIndex = cfg.frames[a->variant][0];
    a->registerCollider();
    return a;
}

std::unique_ptr<Asteroid> Asteroid::makeSmall(int group, int idx, glm::vec2 pos, glm::vec2 vel, float rotSpeed) {
    const auto& cfg = AsteroidSizeConfigs::All[static_cast<int>(AsteroidSize::Small)];
    auto a = std::unique_ptr<Asteroid>(new Asteroid());
    a->pos        = pos;
    a->vel        = vel;
    a->rotSpeed   = rotSpeed;
    a->size         = AsteroidSize::Small;
    a->variant      = group & 3;
    a->m_frameIndex = cfg.frames[a->variant][idx & 3];
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

// ---- audio ------------------------------------------------------------------

void Asteroid::playDestructionSound() const {
    const AsteroidSizeConfig& cfg = AsteroidSizeConfigs::All[static_cast<int>(size)];
    // Decay factor: amplitude reaches ~0.1% of original after noiseFadeTime seconds.
    const float decay = std::pow(0.001f, 1.f / (44100.f * cfg.noiseFadeTime));
    Engine::Services::audio().playNoise(cfg.noiseDuration, cfg.noiseAmplitude, decay);
}

void Asteroid::emitDestructionParticles() const {
    static const glm::vec3 DEBRIS_COLOR = { 0.9f, 0.88f, 0.82f };
    const AsteroidSizeConfig& cfg = AsteroidSizeConfigs::All[static_cast<int>(size)];

    Engine::ParticleEmitParams params;
    params.origin           = pos;
    params.color            = DEBRIS_COLOR;
    params.count            = cfg.particleCount;
    params.speed            = cfg.particleSpeed;
    params.speedVariance    = cfg.particleSpeedVariance;
    params.lifetime         = cfg.particleLifetime;
    params.lifetimeVariance = cfg.particleLifetimeVariance;
    params.startSize        = cfg.particleSize;
    Engine::Services::particles().emit(params);
}

// ---- render -----------------------------------------------------------------

void Asteroid::render(Engine::Renderer2D& renderer, const Engine::SpriteSheet& sheet) const {
    const int   cells      = AsteroidSizeConfigs::All[static_cast<int>(size)].cellCount;
    const float renderSize = cells * 16.f * SCALE;
    const float half       = renderSize * 0.5f;

    const Engine::UVRect uv = sheet.getFrameUVs(m_frameIndex, cells, cells);
    renderer.drawTexturedRect(pos.x - half, pos.y - half, renderSize, renderSize,
                              sheet.texture(),
                              uv.u0, uv.v0, uv.u1, uv.v1,
                              angle);
}
