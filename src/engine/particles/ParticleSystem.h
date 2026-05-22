#pragma once
#include "Particle.h"
#include "ParticleEmitParams.h"
#include <vector>
#include <random>

namespace Engine {

class Renderer2D;

class ParticleSystem {
public:
    explicit ParticleSystem(int capacity = 512);

    void emit(const ParticleEmitParams& params);
    void update(float dt);
    void render(Renderer2D& renderer);
    void clear();

    int activeCount() const { return m_count; }

private:
    std::vector<Particle> m_pool;
    int                   m_count = 0;
    std::mt19937          m_rng;
};

} // namespace Engine
