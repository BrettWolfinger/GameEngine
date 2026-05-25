#pragma once
#include <engine/physics/Collider.h>

namespace Engine::Collision {

// Re-export collision types so game code only needs Engine::Collision::
using ColliderHandle    = Engine::ColliderHandle;
using ColliderDesc      = Engine::ColliderDesc;
using CollisionCallback = Engine::CollisionCallback;
inline constexpr ColliderHandle NULL_COLLIDER = Engine::NULL_COLLIDER;

ColliderHandle add      (const ColliderDesc& desc, CollisionCallback callback);
void           remove   (ColliderHandle handle);
void           updateCircle(ColliderHandle handle, float cx, float cy, float r);
void           updateAABB  (ColliderHandle handle, float x, float y, float w, float h);

} // namespace Engine::Collision
