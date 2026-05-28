# Event System

## Overview

The event system is a typed publish/subscribe bus that decouples systems from each other. Any system can emit an event; any other system can subscribe to it. Neither needs a reference to the other, and neither needs to know the other exists.

The canonical example is audio: `PacmanGame` emits `GhostEaten` when a ghost is caught. `PacmanAudio` subscribes and plays a sound. `PacmanGame` has no audio calls — it just emits events and the audio system reacts independently.

---

## Event Structs

Events are plain data structs. No base class, no registration macro, no boilerplate:

```cpp
// PacmanEvents.h
struct DotEaten         { int col; int row; int points; };
struct GhostEaten       { GhostType type; int multiplier; int points; };
struct PacmanCaught     {};
struct LevelCleared     {};
```

Any struct can be an event. Define them wherever makes sense — typically a `GameEvents.h` alongside the game that owns them.

---

## Subscribing

`Engine::Events::on<T>()` registers a listener and returns a `ListenerHandle`. Store the handle as a member — the subscription stays active as long as the handle is alive, and auto-cancels when the handle is destroyed.

```cpp
class PacmanAudio {
    Engine::ListenerHandle m_ghostHandle;
    Engine::ListenerHandle m_deathHandle;
};

PacmanAudio::PacmanAudio() {
    m_ghostHandle = Engine::Events::on<GhostEaten>([this](const GhostEaten& e) {
        Engine::Audio::playSound(m_eatGhost);
    });

    m_deathHandle = Engine::Events::on<DeathStarted>([this](const DeathStarted&) {
        Engine::Audio::playSound(m_death);
    });
}
```

When `PacmanAudio` is destroyed, its handles go out of scope and both subscriptions are automatically removed. No manual cleanup needed.

---

## Emitting

`Engine::Events::emit()` fires an event immediately. All registered listeners for that type are called synchronously before `emit` returns.

```cpp
void PacmanGame::checkGhostCollision() {
    // ...
    if (ghost.mode() == GhostMode::Frightened) {
        ghost.startEyes();
        Engine::Events::emit(GhostEaten{ ghost.type(), m_ghostsEatenThisPellet, points });
    }
}
```

---

## ListenerHandle Lifetime

`ListenerHandle` is non-copyable and movable. A default-constructed handle is a no-op sentinel.

```cpp
Engine::ListenerHandle m_handle;          // no-op, not subscribed

m_handle = Engine::Events::on<DotEaten>( // now subscribed
    [this](const DotEaten& e) { ... });

m_handle = Engine::Events::on<GhostEaten>( // previous subscription auto-cancelled
    [this](const GhostEaten& e) { ... });

m_handle.reset();                         // explicit early cancel
```

A common mistake is failing to store the handle:

```cpp
Engine::Events::on<DotEaten>([this](const DotEaten& e) { ... }); // BUG: immediately unsubscribes
```

The return value must be stored to keep the subscription alive.

---

## Dispatch Behavior

- **Immediate** — listeners fire synchronously inside `emit()`, in subscription order.
- **Re-entrant safe** — a listener that calls `emit()` or `reset()` during dispatch is handled correctly. Entries unsubscribed during dispatch are marked dead and swept after the outermost `emit` returns; no iterator invalidation occurs.
- **No frame delay** — if you emit in `onUpdate`, listeners respond in the same tick.

---

## Wiring Pattern

The recommended pattern is to wire all subscriptions in one place — typically a constructor or `onInit()`:

```cpp
void PacmanGame::onInit() {
    // ... asset loading, entity construction ...

    m_audio.emplace();   // PacmanAudio subscribes internally

    m_dotHandle = Engine::Events::on<DotEaten>([this](const DotEaten& e) {
        m_score += e.points;
        checkGhostRelease();
    });

    m_levelHandle = Engine::Events::on<LevelCleared>([this](const LevelCleared&) {
        startLevelClear();
    });
}
```

This gives you a single place to read the full event → reaction map for a system, rather than hunting through multiple functions to understand all the consequences of an action.

---

## Adding Events to a Game

1. Define event structs in a `GameEvents.h` file alongside the game
2. In the emitting system, call `Engine::Events::emit(MyEvent{ ... })` at the point the event occurs
3. In the reacting system, call `Engine::Events::on<MyEvent>(callback)` and store the returned handle as a member
4. Include `<engine/Engine.h>` for access to `Engine::Events` and `Engine::ListenerHandle`

---

## Internal Architecture

The event bus is a static facade (`Engine::Events`) backed by an `EventDispatcher` owned by `Application::Impl` and registered with `Services`. This follows the same pattern as `Engine::Audio`, `Engine::Collision`, etc.

Internally, `EventDispatcher` stores listeners in an `unordered_map` keyed by `std::type_index`. The public templates in `Events.h` erase the event type into a `void*` callback and `std::type_index` key at the call site — the only point where `EventT` is known. All downstream dispatch is non-template and routes through two internal functions (`subscribeErased`, `emitErased`).

This means game code never needs to register event types upfront. Any struct is a valid event type.
