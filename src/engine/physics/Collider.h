#pragma once
#include <cstdint>
#include <functional>

namespace Engine {

using ColliderHandle = uint32_t;
inline constexpr ColliderHandle NULL_COLLIDER = 0;

enum class ColliderShape { AABB, Circle };

struct ColliderDesc {
    ColliderShape shape = ColliderShape::Circle;
    uint32_t      layer = 0;
    uint32_t      mask  = 0;
    union {
        struct { float x, y, w, h; } aabb;
        struct { float cx, cy, r;  } circle;
    };

    ColliderDesc() : circle{ 0.f, 0.f, 0.f } {}

    static ColliderDesc makeCircle(uint32_t layer, uint32_t mask, float cx, float cy, float r) {
        ColliderDesc d;
        d.shape     = ColliderShape::Circle;
        d.layer     = layer;
        d.mask      = mask;
        d.circle    = { cx, cy, r };
        return d;
    }

    static ColliderDesc makeAABB(uint32_t layer, uint32_t mask, float x, float y, float w, float h) {
        ColliderDesc d;
        d.shape  = ColliderShape::AABB;
        d.layer  = layer;
        d.mask   = mask;
        d.aabb   = { x, y, w, h };
        return d;
    }
};

using CollisionCallback = std::function<void(ColliderHandle self, ColliderHandle other)>;

} // namespace Engine
