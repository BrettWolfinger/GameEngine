#pragma once
#include <engine/particles/ParticleEmitParams.h>
#include <engine/renderer/Renderer2D.h>

namespace Engine::Particles {

void emit  (const ParticleEmitParams& params);
void update(float dt);
void render(Renderer2D& renderer);
void clear ();

} // namespace Engine::Particles
