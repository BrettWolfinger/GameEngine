#include "CollisionWorld.h"
#include <algorithm>
#include <cmath>

namespace Engine {

ColliderHandle CollisionWorld::add(const ColliderDesc& desc, CollisionCallback callback) {
    const ColliderHandle handle = m_nextHandle++;

    // Reuse an inactive slot before growing the vector.
    for (auto& e : m_entries) {
        if (!e.active) {
            e = { desc, std::move(callback), handle, true };
            return handle;
        }
    }

    m_entries.push_back({ desc, std::move(callback), handle, true });
    return handle;
}

void CollisionWorld::remove(ColliderHandle handle) {
    if (handle == NULL_COLLIDER) return;
    if (auto* e = find(handle))
        e->active = false;
}

void CollisionWorld::updateAABB(ColliderHandle handle, float x, float y, float w, float h) {
    if (auto* e = find(handle))
        e->desc.aabb = { x, y, w, h };
}

void CollisionWorld::updateCircle(ColliderHandle handle, float cx, float cy, float r) {
    if (auto* e = find(handle))
        e->desc.circle = { cx, cy, r };
}

void CollisionWorld::step() {
    for (size_t i = 0; i < m_entries.size(); ++i) {
        auto& a = m_entries[i];
        if (!a.active) continue;

        for (size_t j = i + 1; j < m_entries.size(); ++j) {
            auto& b = m_entries[j];
            if (!b.active) continue;

            // Each side fires its callback only if it listed the other's layer in its mask.
            const bool aWantsB = (b.desc.layer & a.desc.mask) != 0;
            const bool bWantsA = (a.desc.layer & b.desc.mask) != 0;
            if (!aWantsB && !bWantsA) continue;

            if (!overlaps(a.desc, b.desc)) continue;

            if (aWantsB && a.callback) a.callback(a.handle, b.handle);
            // Re-check b.active: a's callback may have called remove() on b.
            if (b.active && bWantsA && b.callback) b.callback(b.handle, a.handle);
        }
    }
}

// ---- private ----------------------------------------------------------------

CollisionWorld::Entry* CollisionWorld::find(ColliderHandle handle) {
    for (auto& e : m_entries)
        if (e.active && e.handle == handle)
            return &e;
    return nullptr;
}

bool CollisionWorld::overlaps(const ColliderDesc& a, const ColliderDesc& b) {
    if (a.shape == ColliderShape::Circle && b.shape == ColliderShape::Circle)
        return circleVsCircle(a, b);
    if (a.shape == ColliderShape::AABB && b.shape == ColliderShape::AABB)
        return aabbVsAabb(a, b);
    // Mixed: normalize so AABB is first argument.
    return a.shape == ColliderShape::AABB ? aabbVsCircle(a, b) : aabbVsCircle(b, a);
}

bool CollisionWorld::circleVsCircle(const ColliderDesc& a, const ColliderDesc& b) {
    const float dx   = a.circle.cx - b.circle.cx;
    const float dy   = a.circle.cy - b.circle.cy;
    const float rSum = a.circle.r  + b.circle.r;
    return dx * dx + dy * dy < rSum * rSum;
}

bool CollisionWorld::aabbVsAabb(const ColliderDesc& a, const ColliderDesc& b) {
    return a.aabb.x              < b.aabb.x + b.aabb.w &&
           a.aabb.x + a.aabb.w  > b.aabb.x &&
           a.aabb.y              < b.aabb.y + b.aabb.h &&
           a.aabb.y + a.aabb.h  > b.aabb.y;
}

bool CollisionWorld::aabbVsCircle(const ColliderDesc& ab, const ColliderDesc& ci) {
    const float nearestX = std::max(ab.aabb.x, std::min(ci.circle.cx, ab.aabb.x + ab.aabb.w));
    const float nearestY = std::max(ab.aabb.y, std::min(ci.circle.cy, ab.aabb.y + ab.aabb.h));
    const float dx = ci.circle.cx - nearestX;
    const float dy = ci.circle.cy - nearestY;
    return dx * dx + dy * dy < ci.circle.r * ci.circle.r;
}

} // namespace Engine
