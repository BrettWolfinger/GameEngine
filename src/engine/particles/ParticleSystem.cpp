#include "ParticleSystem.h"
#include <engine/renderer/Renderer2D.h>
#include <glm/glm.hpp>
#include <cmath>

namespace Engine {

ParticleSystem::ParticleSystem(int capacity)
    : m_rng(std::random_device{}())
{
    m_pool.resize(capacity);
}

void ParticleSystem::emit(const ParticleEmitParams& params) {
    std::uniform_real_distribution<float> angleDist(0.f, 6.28318530f);
    std::uniform_real_distribution<float> speedVar(-params.speedVariance, params.speedVariance);
    std::uniform_real_distribution<float> lifeVar(-params.lifetimeVariance, params.lifetimeVariance);

    for (int i = 0; i < params.count; ++i) {
        if (m_count >= static_cast<int>(m_pool.size())) break;

        const float angle    = angleDist(m_rng);
        const float speed    = params.speed + speedVar(m_rng);
        const float lifetime = std::max(0.05f, params.lifetime + lifeVar(m_rng));

        Particle& p  = m_pool[m_count++];
        p.pos        = params.origin;
        p.vel        = { std::cos(angle) * speed, std::sin(angle) * speed };
        p.color      = params.color;
        p.lifetime   = lifetime;
        p.age        = 0.f;
        p.startSize  = params.startSize;
        p.endSize    = params.endSize;
    }
}

void ParticleSystem::update(float dt) {
    for (int i = 0; i < m_count; ) {
        Particle& p = m_pool[i];
        p.age += dt;
        p.pos += p.vel * dt;

        if (p.age >= p.lifetime) {
            m_pool[i] = m_pool[--m_count]; // swap-remove
        } else {
            ++i;
        }
    }
}

void ParticleSystem::render(Renderer2D& renderer) {
    for (int i = 0; i < m_count; ++i) {
        const Particle& p = m_pool[i];
        const float t     = p.age / p.lifetime;
        const float alpha = 1.f - t;
        const float size  = glm::mix(p.startSize, p.endSize, t);
        if (size <= 0.f) continue;

        const glm::vec4 color = { p.color.r, p.color.g, p.color.b, alpha };
        renderer.drawRect(p.pos.x - size * 0.5f, p.pos.y - size * 0.5f, size, size, color,
                         Renderer2D::kParticleLayer);
    }
}

void ParticleSystem::clear() {
    m_count = 0;
}

} // namespace Engine
