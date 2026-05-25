#include "Particles.h"
#include <engine/core/Services.h>
#include <engine/particles/ParticleSystem.h>

namespace Engine::Particles {

void emit(const ParticleEmitParams& params) {
    Services::particles().emit(params);
}

void clear() {
    Services::particles().clear();
}

} // namespace Engine::Particles
