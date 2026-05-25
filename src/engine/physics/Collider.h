/// @file Collider.h
/// @brief Collision types used directly by game code.
///
/// Game code stores ColliderHandle as a member and constructs ColliderDesc
/// via the static factory methods. Include this file only if you need the
/// types standalone; most game code gets them transitively via
/// `<engine/facade/Collision.h>` or `<engine/Engine.h>`.
#pragma once
#include <cstdint>
#include <functional>

namespace Engine {

/// Opaque handle to a registered collider. Store as a member on any object
/// that needs to update or remove its collider. Compare against NULL_COLLIDER
/// to test whether a collider has been registered.
using ColliderHandle = uint32_t;

/// Sentinel value indicating no collider is registered.
inline constexpr ColliderHandle NULL_COLLIDER = 0;

/// Shape tag used internally by ColliderDesc.
enum class ColliderShape { AABB, Circle };

/// Describes the shape and collision filter for a collider.
/// Construct via the static factory methods rather than initialising fields directly.
///
/// **Layer/mask filtering:** a collision between A and B is reported when
/// `(A.layer & B.mask) || (B.layer & A.mask)`. Assign each object category a
/// power-of-two layer bit and combine categories with bitwise OR to build masks.
struct ColliderDesc {
    ColliderShape shape = ColliderShape::Circle; ///< Active shape variant.
    uint32_t      layer = 0; ///< Category bits for this collider.
    uint32_t      mask  = 0; ///< Which category bits this collider responds to.
    union {
        struct { float x, y, w, h; } aabb;   ///< Valid when shape == AABB.
        struct { float cx, cy, r;  } circle; ///< Valid when shape == Circle.
    };

    ColliderDesc() : circle{ 0.f, 0.f, 0.f } {}

    /// Create a circle collider.
    /// @param layer  Category bits for this collider.
    /// @param mask   Category bits this collider responds to.
    /// @param cx     Centre x in world coordinates.
    /// @param cy     Centre y in world coordinates.
    /// @param r      Radius.
    static ColliderDesc makeCircle(uint32_t layer, uint32_t mask, float cx, float cy, float r) {
        ColliderDesc d;
        d.shape     = ColliderShape::Circle;
        d.layer     = layer;
        d.mask      = mask;
        d.circle    = { cx, cy, r };
        return d;
    }

    /// Create an axis-aligned bounding box collider.
    /// @param layer  Category bits for this collider.
    /// @param mask   Category bits this collider responds to.
    /// @param x      Left edge in world coordinates.
    /// @param y      Top edge in world coordinates.
    /// @param w      Width.
    /// @param h      Height.
    static ColliderDesc makeAABB(uint32_t layer, uint32_t mask, float x, float y, float w, float h) {
        ColliderDesc d;
        d.shape  = ColliderShape::AABB;
        d.layer  = layer;
        d.mask   = mask;
        d.aabb   = { x, y, w, h };
        return d;
    }
};

/// Called by CollisionWorld::step() before pair tests to sync a collider's
/// position with its owning object's current position. Register one at
/// collider creation time instead of calling updateCircle/updateAABB manually
/// each tick. May be nullptr for static colliders that never move.
using SyncFn = std::function<void()>;

/// Callback fired each fixed tick for every overlapping collider pair that
/// passes the layer/mask filter.
/// @param self   Handle of the collider that registered this callback.
/// @param other  Handle of the collider it is overlapping with.
using CollisionCallback = std::function<void(ColliderHandle self, ColliderHandle other)>;

} // namespace Engine
