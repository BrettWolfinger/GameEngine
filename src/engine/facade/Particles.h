#pragma once
#include <engine/core/Services.h>
#include <engine/particles/ParticleSystem.h>
#include <engine/particles/ParticleEmitParams.h>
#include <engine/renderer/Renderer2D.h>

namespace Engine::Particles {

inline void emit(const ParticleEmitParams& params) {
    Services::particles().emit(params);
}

inline void update(float dt) {
    Services::particles().update(dt);
}

inline void render(Renderer2D& renderer) {
    Services::particles().render(renderer);
}

inline void clear() {
    Services::particles().clear();
}

} // namespace Engine::Particles
