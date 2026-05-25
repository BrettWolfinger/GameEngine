# Engine Facade

A stable public API layer between engine internals and game code. Games program
against the facade rather than reaching into engine subsystems directly.
Internal refactors — renaming, restructuring, replacing subsystems — only
require updating the facade, not every game.

---

## Why

Without the facade, game code calls engine internals directly:

```cpp
Engine::Services::audio().playTone(480.f, 0.05f);
Engine::Services::particles().emit(params);
Engine::Services::collision().add(desc, callback);
```

`Services` is an internal registry. Exposing it to games means any refactor of
how the engine manages subsystems forces fixes across every game file. This is
what broke Pong and Breakout when `AudioManager` changed from static to instance
methods.

---

## Design

### Free functions in `Engine::<Subsystem>` namespaces

Games call free functions grouped by subsystem namespace. The facade owns the
mapping to the current internal implementation.

```cpp
// Game code — stable across engine refactors
Engine::Audio::playTone(480.f, 0.05f);
Engine::Particles::emit(params);
Engine::Collision::add(desc, callback);
```

Sub-namespaces keep the flat `Engine` namespace from crowding as more subsystems
are added, and make call sites self-documenting without exposing implementation
details.

### One header per subsystem, one aggregating header

```
src/engine/facade/
    Audio.h       → Engine::Audio::*
    Particles.h   → Engine::Particles::*
    Collision.h   → Engine::Collision::*
    Config.h      → Engine::Config::*

src/engine/Engine.h   ← most game code includes this
```

Each facade header is a pure declaration file — no implementation, no internal
includes. The corresponding `.cpp` in the same directory holds the
implementation and is the only place that includes engine-internal headers.

`Engine.h` is a thin aggregator:

```cpp
#pragma once
#include <engine/facade/Audio.h>
#include <engine/facade/Collision.h>
#include <engine/facade/Config.h>
#include <engine/facade/Particles.h>
```

Game `.cpp` files that use more than one subsystem include `<engine/Engine.h>`.
Game `.h` files that need a specific type (e.g. `ColliderHandle` as a member)
include the per-subsystem header directly.

### What goes in the facade

Operations — things games *do*:

- Play audio
- Emit particles (`emit`, `clear` — `update` and `render` are engine-owned)
- Register, update, and remove colliders
- Register hot-reload callbacks for config files

### What does not go in the facade

Types that games hold or pass around stay as direct includes — they cannot be
hidden behind the facade without forcing games to use opaque handles for
everything:

- `glm::vec2`, `glm::vec4` — math types
- `ColliderHandle` — stored as a member on game objects
- `ParticleEmitParams` — constructed by game code
- `Engine::Renderer2D` — passed to `render()` by the engine, not requested by games

### Enforcement

Internal headers are guarded with `ENGINE_INTERNAL`:

```cpp
#pragma once
#ifndef ENGINE_INTERNAL
#  error "engine/core/Services.h is an engine-internal header. Include <engine/Engine.h> instead."
#endif
```

The engine CMake target defines `ENGINE_INTERNAL` privately, so only engine
translation units can include these headers. Game code that tries to include
`Services.h`, `AudioManager.h`, `ParticleSystem.h`, or `CollisionWorld.h`
directly gets a compile error with a clear message.

`Application` stores its subsystem members (`AudioManager`, `ParticleSystem`,
`CollisionWorld`) behind a pimpl (`struct Application::Impl` in
`Application.cpp`), so `Application.h` needs no internal includes and leaks
nothing to games that subclass it.

---

## Adding a new subsystem

1. Create `src/engine/facade/MySystem.h` — declarations only, no internal includes.
   Add Doxygen `///` comments for every function.
2. Create `src/engine/facade/MySystem.cpp` — include `Services.h` and the
   internal subsystem header here, implement the wrappers.
3. Add the `.cpp` to `src/engine/CMakeLists.txt`.
4. Add `#include <engine/facade/MySystem.h>` to `src/engine/Engine.h`.
5. Add `ENGINE_INTERNAL` guard to the internal subsystem header.
6. If the subsystem is owned by `Application`, add it to `struct Application::Impl`
   in `Application.cpp` and register it with `Services` in the constructor.

---

## Doxygen

All facade headers and shared game types are annotated with Doxygen `///`
comments. Update comments in the same commit as any signature change — stale
parameter docs are worse than none.

Annotate:
- `src/engine/facade/*.h` and `src/engine/Engine.h`
- Shared types games construct directly: `ParticleEmitParams.h`, `Collider.h`

Do not annotate engine internals (`Services`, `AudioManager`, etc.).

CI config and GitHub Pages publishing are tracked in issue #68.

---

## Benefits

| Concern | Without facade | With facade |
|---|---|---|
| Internal refactor scope | All game files | Facade only |
| New developer onboarding | Read engine source | Read `Engine.h` |
| "What can a game do?" | Implicit, scattered | Explicit, one place |
| Breaking change detection | Build failure | Build failure (same) |
| Breaking change *blast radius* | All games | Zero games |

---

## Call site reference

```cpp
// Audio
Engine::Audio::playTone(480.f, 0.05f);
Engine::Audio::playNoise(0.40f, 0.40f, decay);
Engine::Audio::playLoopingTone(slot, hz, amplitude);
Engine::Audio::playLoopingNoise(slot, amplitude);
Engine::Audio::stopLoopingVoice(slot);

// Particles — update/render are engine-owned; games only call emit and clear
Engine::Particles::emit(params);
Engine::Particles::clear();  // call on state transitions to prevent stale particles

// Collision
Engine::Collision::add(desc, callback);
Engine::Collision::remove(handle);
Engine::Collision::updateCircle(handle, cx, cy, r);
Engine::Collision::updateAABB(handle, x, y, w, h);

// Config — hot-reload (no-op in release builds, so no #ifdef needed at call sites)
Engine::Config::watch("games/mygame/assets/configs/enemies.toml", loadEnemyConfigs);
```
