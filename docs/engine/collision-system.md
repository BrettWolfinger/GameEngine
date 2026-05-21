# Collision System

## Overview

The collision system lets game objects detect overlaps and react to them without any game-level polling. Objects self-register when constructed, deregister when destroyed, and receive callbacks when a collision is detected. The engine drives the timing; game code only handles the response.

---

## Layers and Masks

Every collider declares two bitmask values:

- **layer** — what type of object this is
- **mask** — which layers this object wants to hear about

```cpp
// AsteroidsConfig.h
inline constexpr uint32_t kBulletLayer   = 1 << 0;
inline constexpr uint32_t kAsteroidLayer = 1 << 1;
```

A callback fires for object A only if B's layer appears in A's mask. Both sides opt in independently:

| A's mask includes B's layer | B's mask includes A's layer | Result |
|---|---|---|
| yes | yes | Both callbacks fire |
| yes | no | Only A's callback fires |
| no | yes | Only B's callback fires |
| no | no | Pair skipped entirely |

This means each object only responds to what it cares about. A bullet hitting an asteroid fires the bullet's callback (kills the bullet) and the asteroid's callback (marks it shot). If we later add ship-asteroid collision where the asteroid shouldn't respond differently, the asteroid simply doesn't include `kShipLayer` in its mask.

---

## Handles and ColliderDesc

`ColliderHandle` is an opaque integer returned by `CollisionWorld::add()`. It's used to update position and remove the collider. `NULL_COLLIDER` (0) is the sentinel for an unregistered state.

Shapes are described via `ColliderDesc` static factories:

```cpp
ColliderDesc::makeCircle(layer, mask, cx, cy, radius)
ColliderDesc::makeAABB(layer, mask, x, y, w, h)
```

Mixed pairs (circle vs AABB) are supported — `CollisionWorld` normalizes the order internally.

---

## Self-Registration

Game objects register their own colliders in the constructor and deregister in the destructor. No game-level lifetime management is needed.

```cpp
void Asteroid::registerCollider() {
    m_colliderHandle = Engine::Services::collision().add(
        Engine::ColliderDesc::makeCircle(kAsteroidLayer, kBulletLayer, pos.x, pos.y, radius()),
        [this](Engine::ColliderHandle self, Engine::ColliderHandle) {
            m_wasShot = true;
            Engine::Services::collision().remove(self);
            m_colliderHandle = Engine::NULL_COLLIDER;
        });
}

Asteroid::~Asteroid() {
    Engine::Services::collision().remove(m_colliderHandle);
}
```

The destructor call is always safe — `remove(NULL_COLLIDER)` is a no-op. If the callback already removed the collider, the destructor does nothing.

---

## Services Locator

`Engine::Services::collision()` gives any object access to the `CollisionWorld` without threading a reference through constructors. `Application` owns the `CollisionWorld` instance and registers it with `Services` on startup:

```cpp
// Application.cpp
Application::Application(...) {
    Services::setCollision(&m_collisionWorld);
}
```

Game objects include `<engine/core/Services.h>` and call `Services::collision()` directly — no pointer to the game or `Application` needed.

---

## Tick Order

Each fixed tick runs in this sequence:

```
Input::update()
preStep(dt)     <- game moves objects and syncs collider positions
step()          <- overlaps tested, callbacks fire (flags set, handles nulled)
onUpdate(dt)    <- game reads flags, spawns fragments, erases dead objects
onRender()
```

`preStep()` is a virtual on `Application` that games override to move all objects before collision is tested. This ensures `step()` always sees current-tick positions with no one-tick lag.

---

## Position Sync

Collider positions must be kept in sync with game object positions inside each object's `update()` call:

```cpp
void Asteroid::update(float dt, int screenW, int screenH) {
    pos   += vel * dt;
    // ... wrapping ...

    if (m_colliderHandle != Engine::NULL_COLLIDER)
        Engine::Services::collision().updateCircle(m_colliderHandle, pos.x, pos.y, radius());
}
```

The `NULL_COLLIDER` guard matters — the collision callback nulls the handle after removing the collider. Without the guard, a dead asteroid would try to update a stale handle.

---

## Callback Safety

Two rules keep `this`-capturing callbacks safe:

1. **Callbacks only set flags and remove their own collider.** They never erase objects from vectors or spawn new ones. All container mutation is deferred to `onUpdate()`, which runs after `step()` returns.

2. **`remove()` is safe to call during dispatch.** It sets `active = false` without touching the entries vector, so no iterator invalidation occurs inside `step()`. `CollisionWorld` re-checks `b.active` before firing b's callback in case a's callback already removed b.

---

## Adding a New Collideable Object

1. Add a layer constant to the game's config header
2. Store a `ColliderHandle m_colliderHandle = Engine::NULL_COLLIDER` member
3. Call `Services::collision().add(...)` in the constructor with a `this`-capturing callback
4. Call `Services::collision().remove(m_colliderHandle)` in the destructor
5. Call `updateCircle/updateAABB` at the end of `update()`, guarded by `!= NULL_COLLIDER`
6. Call `object.update(dt)` from the game's `preStep()` override so positions sync before `step()` runs
