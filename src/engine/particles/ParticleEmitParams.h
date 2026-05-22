#pragma once
#include <glm/glm.hpp>

namespace Engine {

struct ParticleEmitParams {
    glm::vec2 origin;
    glm::vec3 color          = { 1.f, 1.f, 1.f };
    int       count          = 8;
    float     speed          = 100.f;
    float     speedVariance  = 40.f;
    float     lifetime       = 1.f;
    float     lifetimeVariance = 0.2f;
    float     startSize      = 4.f;
    float     endSize        = 0.f;
};

} // namespace Engine
