/// @file Particles.h
/// @brief Engine particle facade — emit and manage particle effects.
///
/// Game code constructs a ParticleEmitParams and calls emit().
/// update() and render() are called automatically by the engine loop
/// and do not need to be called from game code.
#pragma once
#include <engine/particles/ParticleEmitParams.h>
#include <engine/renderer/Renderer2D.h>

namespace Engine::Particles {

/// Emit a burst of particles.
/// @param params  Description of the burst — origin, color, count, speed, and lifetime.
///                See ParticleEmitParams for field documentation.
void emit(const ParticleEmitParams& params);

/// Advance all active particles by one timestep.
/// Called automatically by the engine loop — do not call from game code.
/// @param dt  Fixed timestep in seconds (1/60).
void update(float dt);

/// Draw all active particles.
/// Called automatically by the engine loop — do not call from game code.
/// @param renderer  The frame's 2D renderer.
void render(Renderer2D& renderer);

/// Immediately remove all active particles.
/// Useful when transitioning between game states.
void clear();

} // namespace Engine::Particles
