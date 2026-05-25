#pragma once
#ifndef ENGINE_INTERNAL
#  error "engine/physics/CollisionWorld.h is an engine-internal header. Include <engine/Engine.h> instead."
#endif
#include "Collider.h"
#include <vector>

namespace Engine {

class CollisionWorld {
public:
    // Register a collider. Returns an opaque handle used for updates and removal.
    // Callbacks must not remove game objects from containers during dispatch;
    // defer side effects (removal, spawning) until after step() returns.
    ColliderHandle add(const ColliderDesc& desc, CollisionCallback callback);

    // Deregister a collider. Safe to call with NULL_COLLIDER (no-op).
    void remove(ColliderHandle handle);

    void updateAABB  (ColliderHandle handle, float x, float y, float w, float h);
    void updateCircle(ColliderHandle handle, float cx, float cy, float r);

    // Test all active pairs and fire callbacks for overlapping colliders.
    // Called once per fixed tick by Application, before onUpdate.
    void step();

private:
    struct Entry {
        ColliderDesc      desc;
        CollisionCallback callback;
        ColliderHandle    handle = NULL_COLLIDER;
        bool              active = false;
    };

    std::vector<Entry> m_entries;
    ColliderHandle     m_nextHandle = 1; // 0 reserved for NULL_COLLIDER

    Entry* find(ColliderHandle handle);

    static bool overlaps      (const ColliderDesc& a, const ColliderDesc& b);
    static bool circleVsCircle(const ColliderDesc& a, const ColliderDesc& b);
    static bool aabbVsAabb    (const ColliderDesc& a, const ColliderDesc& b);
    static bool aabbVsCircle  (const ColliderDesc& ab, const ColliderDesc& ci);
};

} // namespace Engine
