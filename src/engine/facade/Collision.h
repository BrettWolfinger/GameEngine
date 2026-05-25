/// @file Collision.h
/// @brief Engine collision facade — register, update, and remove colliders.
///
/// Colliders are identified by an opaque ColliderHandle. Store the handle
/// as a member on any object that needs to update or remove its collider.
/// Use NULL_COLLIDER as a sentinel for "no collider registered."
///
/// Layer/mask filtering: a collision is reported when
/// `(a.layer & b.mask) || (b.layer & a.mask)`. Assign layers as powers of
/// two and combine them with bitwise OR to build masks.
#pragma once
#include <engine/physics/Collider.h>

namespace Engine::Collision {

// Re-export collision types so game code only needs Engine::Collision::
using ColliderHandle    = Engine::ColliderHandle;   ///< Opaque collider identifier.
using ColliderDesc      = Engine::ColliderDesc;     ///< Collider shape and filter data.
using CollisionCallback = Engine::CollisionCallback;///< `void(ColliderHandle self, ColliderHandle other)`

/// Sentinel value for an unregistered or removed collider.
inline constexpr ColliderHandle NULL_COLLIDER = Engine::NULL_COLLIDER;

/// Register a collider and return its handle.
/// The callback fires each fixed tick for every overlapping pair that passes
/// the layer/mask filter. Do not remove game objects from containers inside
/// the callback — defer side effects until after the tick completes.
/// @param desc      Shape, position, layer, and mask for this collider.
/// @param callback  Called with (self, other) for each overlapping collider.
/// @return          An opaque handle used for updates and removal.
ColliderHandle add(const ColliderDesc& desc, CollisionCallback callback);

/// Deregister a collider. Safe to call with NULL_COLLIDER (no-op).
/// @param handle  Handle returned by add().
void remove(ColliderHandle handle);

/// Update the position and radius of a circle collider.
/// @param handle  Handle returned by add().
/// @param cx      New centre x in world coordinates.
/// @param cy      New centre y in world coordinates.
/// @param r       New radius.
void updateCircle(ColliderHandle handle, float cx, float cy, float r);

/// Update the position and size of an AABB collider.
/// @param handle  Handle returned by add().
/// @param x       New left edge in world coordinates.
/// @param y       New top edge in world coordinates.
/// @param w       New width.
/// @param h       New height.
void updateAABB(ColliderHandle handle, float x, float y, float w, float h);

} // namespace Engine::Collision
