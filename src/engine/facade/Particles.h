/// @file Particles.h
/// @brief Engine particle facade — emit and manage particle effects.
///
/// Game code constructs a ParticleEmitParams and calls emit(). The engine
/// loop automatically ticks and renders particles each frame — games do not
/// call update() or render() directly.
///
/// To enable automatic particle rendering, override Application::getRenderer()
/// in your game class to return a pointer to your Renderer2D instance.
/// Particles render after onRender() and before onOverlayRender().
///
/// Call clear() explicitly when transitioning between game states to prevent
/// stale particles carrying over.
#pragma once
#include <engine/particles/ParticleEmitParams.h>

namespace Engine::Particles {

/// Emit a burst of particles.
/// @param params  Description of the burst — origin, color, count, speed, and lifetime.
///                See ParticleEmitParams for field documentation.
void emit(const ParticleEmitParams& params);

/// Immediately remove all active particles.
/// Useful when transitioning between game states.
void clear();

} // namespace Engine::Particles
