#include "UFO.h"
#include <engine/Engine.h>
#include <engine/core/Services.h>
#include <engine/physics/CollisionWorld.h>
#include <cmath>

UFO::UFO(UfoSize size, glm::vec2 pos, glm::vec2 vel, std::mt19937& rng)
    : m_size(size), m_pos(pos), m_vel(vel), m_rng(rng)
{
    m_colliderHandle = Engine::Services::collision().add(
        Engine::ColliderDesc::makeCircle(kUfoLayer, kBulletLayer, pos.x, pos.y, collisionRadius()),
        [this](Engine::ColliderHandle self, Engine::ColliderHandle) {
            m_wasDestroyed = true;
            emitDestructionParticles();
            const float decay = std::pow(0.001f, 1.f / (44100.f * UfoConfigs::NOISE_FADE_TIME));
            Engine::Audio::playNoise(UfoConfigs::NOISE_DURATION, UfoConfigs::NOISE_AMPLITUDE, decay);
            Engine::Services::collision().remove(self);
            m_colliderHandle = Engine::NULL_COLLIDER;
        });
}

UFO::~UFO() {
    Engine::Services::collision().remove(m_colliderHandle);
}

bool UFO::hasExited(int screenW, int /*screenH*/) const {
    const float margin = renderSize() + 10.f;
    return m_pos.x < -margin || m_pos.x > screenW + margin;
}

void UFO::update(float dt, int screenW, int screenH, glm::vec2 /*shipPos*/) {
    m_zigzagTimer -= dt;
    if (m_zigzagTimer <= 0.f) {
        m_zigzagTimer = UfoConfigs::ZIGZAG_INTERVAL;
        const float ySpeed = std::uniform_real_distribution<float>(
            UfoConfigs::ZIGZAG_MIN_YSPEED, UfoConfigs::ZIGZAG_MAX_YSPEED)(m_rng);
        m_vel.y = (m_vel.y >= 0.f) ? -ySpeed : ySpeed;
    }

    m_pos += m_vel * dt;

    const float rs = renderSize();
    if (m_pos.y < -rs)          m_pos.y += screenH + 2.f * rs;
    if (m_pos.y > screenH + rs) m_pos.y -= screenH + 2.f * rs;

    m_beepTimer -= dt;
    if (m_beepTimer <= 0.f) {
        m_beepTimer = UfoConfigs::BEEP_INTERVAL;
        const float freq = m_beepHigh ? UfoConfigs::BEEP_FREQUENCY_HI : UfoConfigs::BEEP_FREQUENCY_LO;
        Engine::Audio::playTone(freq, UfoConfigs::BEEP_DURATION, UfoConfigs::BEEP_AMPLITUDE);
        m_beepHigh = !m_beepHigh;
    }

    m_fireTimer -= dt;

    if (m_colliderHandle != Engine::NULL_COLLIDER)
        Engine::Services::collision().updateCircle(m_colliderHandle, m_pos.x, m_pos.y, collisionRadius());
}

std::optional<UFO::BulletSpawn> UFO::tryFire(glm::vec2 shipPos) {
    const UfoConfig& cfg = UfoConfigs::All[static_cast<int>(m_size)];
    if (m_fireTimer > 0.f || m_wasDestroyed) return std::nullopt;
    m_fireTimer = cfg.fireRate;

    // Rotating toShip by a uniform angle in [-aimVariance, +aimVariance] produces
    // a direction in a cone around the ship. When aimVariance ~= pi the result is
    // a fully random direction regardless of ship position (Large UFO behaviour).
    const glm::vec2 toShip   = glm::normalize(shipPos - m_pos);
    const float     variance = std::uniform_real_distribution<float>(-cfg.aimVariance, cfg.aimVariance)(m_rng);
    const float     c = std::cos(variance), s = std::sin(variance);
    const glm::vec2 dir = { c * toShip.x - s * toShip.y, s * toShip.x + c * toShip.y };

    return BulletSpawn{ m_pos, dir };
}

void UFO::render(Engine::Renderer2D& renderer, const Engine::SpriteSheet& sheet) const {
    const UfoConfig& cfg  = UfoConfigs::All[static_cast<int>(m_size)];
    const float      rs   = renderSize();
    const float      half = rs * 0.5f;
    const Engine::UVRect uv = sheet.getFrameUVs(cfg.spriteFrame, UfoConfigs::SPRITE_CELLS, UfoConfigs::SPRITE_CELLS);
    renderer.drawTexturedRect(m_pos.x - half, m_pos.y - half, rs, rs,
                              sheet.texture(), uv.u0, uv.v0, uv.u1, uv.v1);
}

void UFO::emitDestructionParticles() const {
    const UfoConfig& cfg = UfoConfigs::All[static_cast<int>(m_size)];
    Engine::ParticleEmitParams params;
    params.origin           = m_pos;
    params.color            = { 0.3f, 0.95f, 0.95f };
    params.count            = cfg.particleCount;
    params.speed            = cfg.particleSpeed;
    params.speedVariance    = cfg.particleSpeedVariance;
    params.lifetime         = cfg.particleLifetime;
    params.lifetimeVariance = cfg.particleLifetimeVariance;
    params.startSize        = cfg.particleSize;
    params.endSize          = 0.f;
    Engine::Particles::emit(params);
}
