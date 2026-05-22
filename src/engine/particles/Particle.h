#pragma once
#include <glm/glm.hpp>

namespace Engine {

struct Particle {
    glm::vec2 pos;
    glm::vec2 vel;
    glm::vec3 color;
    float     lifetime  = 1.f;
    float     age       = 0.f;
    float     startSize = 4.f;
    float     endSize   = 0.f;
};

} // namespace Engine
