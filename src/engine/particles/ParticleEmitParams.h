/// @file ParticleEmitParams.h
/// @brief Parameters for a single particle burst passed to Engine::Particles::emit().
#pragma once
#include <glm/glm.hpp>

namespace Engine {

/// Describes a burst of particles emitted from a single point.
/// Construct one of these and pass it to Engine::Particles::emit().
struct ParticleEmitParams {
    glm::vec2 origin;                   ///< Emission point in world coordinates.
    glm::vec3 color          = { 1.f, 1.f, 1.f }; ///< RGB color applied to all particles in the burst.
    int       count          = 8;       ///< Number of particles to spawn.
    float     speed          = 100.f;   ///< Base speed in pixels per second.
    float     speedVariance  = 40.f;    ///< Random ± speed deviation per particle.
    float     lifetime       = 1.f;     ///< Base lifetime in seconds.
    float     lifetimeVariance = 0.2f;  ///< Random ± lifetime deviation per particle.
    float     startSize      = 4.f;     ///< Particle size at spawn (pixels).
    float     endSize        = 0.f;     ///< Particle size at end of life (pixels). Interpolated linearly.
};

} // namespace Engine
