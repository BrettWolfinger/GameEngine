#include "UFO.h"
#include <engine/audio/AudioManager.h>
#include <engine/core/Services.h>
#include <engine/particles/ParticleSystem.h>
#include <engine/physics/CollisionWorld.h>
#include <glm/gtc/constants.hpp>
#include <cmath>

UFO::UFO(UfoSize size, glm::vec2 pos, glm::vec2 vel, std::mt19937& rng)
    : m_size(size), m_pos(pos), m_vel(vel), m_rng(rng)
{
    m_colliderHandle = Engine::Services::collision().add(
        Engine::ColliderDesc::makeCircle(kUfoLayer, kBulletLayer, pos.x, pos.y, collisionRadius()),
        [this](Engine::ColliderHandle self, Engine::ColliderHandle) {
            m_wasDestroyed = true;
            emitDestructionParticles();
            Engine::Services::audio().playNoise(0.4f, 0.4f, std::pow(0.001f, 1.f / (44100.f * 0.35f)));
            Engine::Services::collision().remove(self);
            m_colliderHandle = Engine::NULL_COLLIDER;
        });
}

UFO::~UFO() {
    Engine::Services::collision().remove(m_colliderHandle);
}

float UFO::renderSize() const {
    return (m_size == UfoSize::Large) ? LARGE_RENDER_SIZE : SMALL_RENDER_SIZE;
}

float UFO::collisionRadius() const {
    return renderSize() * 0.45f;
}

bool UFO::hasExited(int screenW, int /*screenH*/) const {
    const float margin = renderSize() + 10.f;
    return m_pos.x < -margin || m_pos.x > screenW + margin;
}

void UFO::update(float dt, int screenW, int screenH, glm::vec2 /*shipPos*/) {
    m_zigzagTimer -= dt;
    if (m_zigzagTimer <= 0.f) {
        m_zigzagTimer = ZIGZAG_INTERVAL;
        const float ySpeed = std::uniform_real_distribution<float>(20.f, ZIGZAG_MAX_YSPEED)(m_rng);
        m_vel.y = (m_vel.y >= 0.f) ? -ySpeed : ySpeed;
    }

    m_pos += m_vel * dt;

    const float rs = renderSize();
    if (m_pos.y < -rs)          m_pos.y += screenH + 2.f * rs;
    if (m_pos.y > screenH + rs) m_pos.y -= screenH + 2.f * rs;

    m_beepTimer -= dt;
    if (m_beepTimer <= 0.f) {
        m_beepTimer = BEEP_INTERVAL;
        Engine::Services::audio().playTone(m_beepHigh ? 550.f : 400.f, 0.25f, 0.18f);
        m_beepHigh = !m_beepHigh;
    }

    m_fireTimer -= dt;

    if (m_colliderHandle != Engine::NULL_COLLIDER)
        Engine::Services::collision().updateCircle(m_colliderHandle, m_pos.x, m_pos.y, collisionRadius());
}

std::optional<UFO::BulletSpawn> UFO::tryFire(glm::vec2 shipPos) {
    if (m_fireTimer > 0.f || m_wasDestroyed) return std::nullopt;
    m_fireTimer = (m_size == UfoSize::Large) ? LARGE_FIRE_RATE : SMALL_FIRE_RATE;

    glm::vec2 dir;
    if (m_size == UfoSize::Large) {
        const float angle = std::uniform_real_distribution<float>(0.f, glm::two_pi<float>())(m_rng);
        dir = { std::cos(angle), std::sin(angle) };
    } else {
        const glm::vec2 toShip = glm::normalize(shipPos - m_pos);
        const float variance = std::uniform_real_distribution<float>(-SMALL_AIM_VARIANCE, SMALL_AIM_VARIANCE)(m_rng);
        const float c = std::cos(variance), s = std::sin(variance);
        dir = { c * toShip.x - s * toShip.y, s * toShip.x + c * toShip.y };
    }

    return BulletSpawn{ m_pos, dir };
}

void UFO::render(Engine::Renderer2D& renderer) const {
    static constexpr glm::vec4 COLOR = { 0.3f, 0.95f, 0.95f, 1.f };
    const float rs   = renderSize();
    const float half = rs * 0.5f;

    renderer.drawRect(m_pos.x - half,        m_pos.y - rs * 0.2f, rs,   rs * 0.4f, COLOR);
    renderer.drawRect(m_pos.x - half * 0.5f, m_pos.y - rs * 0.5f, half, rs * 0.3f, COLOR);
}

void UFO::emitDestructionParticles() const {
    Engine::ParticleEmitParams params;
    params.origin           = m_pos;
    params.color            = { 0.3f, 0.95f, 0.95f };
    params.count            = (m_size == UfoSize::Large) ? 18 : 10;
    params.speed            = (m_size == UfoSize::Large) ? 150.f : 110.f;
    params.speedVariance    = 70.f;
    params.lifetime         = (m_size == UfoSize::Large) ? 1.0f : 0.7f;
    params.lifetimeVariance = 0.25f;
    params.startSize        = (m_size == UfoSize::Large) ? 4.f * SCALE : 3.f * SCALE;
    params.endSize          = 0.f;
    Engine::Services::particles().emit(params);
}
