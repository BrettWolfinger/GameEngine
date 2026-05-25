#pragma once
#include <engine/core/Services.h>
#include <engine/physics/Collider.h>
#include <engine/physics/CollisionWorld.h>

namespace Engine::Collision {

// Re-export collision types so game code only needs Engine::Collision::
using ColliderHandle    = Engine::ColliderHandle;
using ColliderDesc      = Engine::ColliderDesc;
using CollisionCallback = Engine::CollisionCallback;
inline constexpr ColliderHandle NULL_COLLIDER = Engine::NULL_COLLIDER;

inline ColliderHandle add(const ColliderDesc& desc, CollisionCallback callback) {
    return Services::collision().add(desc, std::move(callback));
}

inline void remove(ColliderHandle handle) {
    Services::collision().remove(handle);
}

inline void updateCircle(ColliderHandle handle, float cx, float cy, float r) {
    Services::collision().updateCircle(handle, cx, cy, r);
}

inline void updateAABB(ColliderHandle handle, float x, float y, float w, float h) {
    Services::collision().updateAABB(handle, x, y, w, h);
}

} // namespace Engine::Collision
