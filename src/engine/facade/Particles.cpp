#include "Particles.h"
#include <engine/core/Services.h>
#include <engine/particles/ParticleSystem.h>

namespace Engine::Particles {

void emit(const ParticleEmitParams& params) {
    Services::particles().emit(params);
}

void update(float dt) {
    Services::particles().update(dt);
}

void render(Renderer2D& renderer) {
    Services::particles().render(renderer);
}

void clear() {
    Services::particles().clear();
}

} // namespace Engine::Particles
