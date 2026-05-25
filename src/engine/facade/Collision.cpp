#include "Collision.h"
#include <engine/core/Services.h>
#include <engine/physics/CollisionWorld.h>

namespace Engine::Collision {

ColliderHandle add(const ColliderDesc& desc, SyncFn syncFn, CollisionCallback callback) {
    return Services::collision().add(desc, std::move(syncFn), std::move(callback));
}

void remove(ColliderHandle handle) {
    Services::collision().remove(handle);
}

void updateCircle(ColliderHandle handle, float cx, float cy, float r) {
    Services::collision().updateCircle(handle, cx, cy, r);
}

void updateAABB(ColliderHandle handle, float x, float y, float w, float h) {
    Services::collision().updateAABB(handle, x, y, w, h);
}

} // namespace Engine::Collision
