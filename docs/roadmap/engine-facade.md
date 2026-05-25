# Engine Facade

Introduce a stable public API layer between the engine internals and game code.
Games program against the facade rather than reaching into engine subsystems
directly. Internal refactors — renaming, restructuring, replacing subsystems —
only require updating the facade, not every game.

---

## Problem

Game code currently calls engine internals directly:

```cpp
Engine::Services::audio().playTone(480.f, 0.05f);
Engine::Services::particles().emit(params);
Engine::Services::collision().add(desc, callback);
```

`Services` is an internal registry. Exposing it to games means any refactor of
how the engine manages subsystems (e.g. replacing `Services` with a
`SubsystemManager`) forces fixes across all game files. This is what broke Pong
and Breakout when `AudioManager` changed from static to instance methods.

---

## Design

### Stable free functions in `Engine::<Subsystem>` namespaces

Games call free functions grouped by subsystem namespace. The facade owns the
mapping to whatever the current internal implementation is.

```cpp
// Game code — stable across engine refactors
Engine::Audio::playTone(480.f, 0.05f);
Engine::Particles::emit(params);
Engine::Collision::add(desc, callback);
```

```cpp
// Facade implementation — changes here, not in game code
namespace Engine::Audio {
    inline void playTone(float hz, float dur, float amp = 0.4f) {
        Services::audio().playTone(hz, dur, amp);
    }
}
```

Sub-namespaces keep the flat `Engine` namespace from getting crowded as more
subsystems are added, and make call sites self-documenting without exposing any
internal implementation detail.

### One file per subsystem, one aggregating header

```
src/engine/facade/
    Audio.h       → Engine::Audio::playTone, playNoise
    Particles.h   → Engine::Particles::emit
    Collision.h   → Engine::Collision::add, remove, updateCircle

src/engine/Engine.h   ← games include this
```

`Engine.h` is a thin aggregator — no implementation, just includes:

```cpp
#pragma once
#include <engine/facade/Audio.h>
#include <engine/facade/Collision.h>
#include <engine/facade/Particles.h>
```

Games that need only one subsystem can include the per-subsystem header
directly. Most games will just use `Engine.h`.

### What goes in the facade

Operations — things games *do*:

- Play audio
- Emit particles
- Register/update/remove colliders
- (Future) Load assets, log, access save data

### What does not go in the facade

Types that games need to hold or pass around cannot be hidden behind the facade:

- `glm::vec2`, `glm::vec4` — math types, games use these directly
- `ColliderHandle` — games store these as members
- `ParticleEmitParams` — games construct these
- `Engine::Renderer2D` — passed to `render()` by the engine, not requested by games

These stay as direct includes. The facade is for operations, not types.

### Convention, not enforcement

Nothing technically prevents a game from including `<engine/core/Services.h>`
directly and bypassing the facade. The goal is a clear convention:

> Game code includes `<engine/Engine.h>` (or per-subsystem facade headers).
> Game code never includes `<engine/core/Services.h>`.

Enforcing this at the compiler level would require moving `Services.h` out of
the public include path — possible, but a larger restructure to do later once
the facade is established.

---

## Subsystem Phases

Introduce facades incrementally as each subsystem is touched, not all at once.

### Phase 1 — Audio (immediate)

Smallest surface, clearest need — Pong and Breakout already needed fixing here.

**`engine/facade/Audio.h`:**

```cpp
#pragma once
#include <engine/core/Services.h>
#include <engine/audio/AudioManager.h>

namespace Engine::Audio {

inline void playTone(float frequencyHz, float durationSec, float amplitude = 0.4f) {
    Services::audio().playTone(frequencyHz, durationSec, amplitude);
}

inline void playNoise(float durationSec, float amplitude = 0.4f, float decayFactor = 1.f) {
    Services::audio().playNoise(durationSec, amplitude, decayFactor);
}

} // namespace Engine::Audio
```

**Files to update:** `PongGame.cpp`, `BreakoutGame.cpp`, `Asteroid.cpp`, `UFO.cpp`, `Ship.cpp` ✓ Done

Also includes `playLoopingTone`, `playLoopingNoise`, and `stopLoopingVoice` discovered during Phase 2.

---

### Phase 2 — Particles

Simple emit-only surface. `ParticleEmitParams` stays as a direct include since
games construct it.

**`engine/facade/Particles.h`:**

```cpp
#pragma once
#include <engine/core/Services.h>
#include <engine/particles/ParticleSystem.h>

namespace Engine::Particles {

inline void emit(const ParticleEmitParams& params) {
    Services::particles().emit(params);
}

} // namespace Engine::Particles
```

**Files to update:** `Asteroid.cpp`, `UFO.cpp`, `Ship.cpp`, `AsteroidsGame.cpp` ✓ Done

---

### Phase 3 — Collision

Most complex surface. Games register colliders, update positions, and remove
them — often storing `ColliderHandle` as a member. `ColliderHandle` and
`ColliderDesc` remain direct includes.

**`engine/facade/Collision.h`:**

```cpp
#pragma once
#include <engine/core/Services.h>
#include <engine/physics/Collider.h>

namespace Engine::Collision {

inline ColliderHandle add(const ColliderDesc& desc, CollisionCallback callback) {
    return Services::collision().add(desc, std::move(callback));
}

inline void remove(ColliderHandle handle) {
    Services::collision().remove(handle);
}

inline void updateCircle(ColliderHandle handle, float x, float y, float radius) {
    Services::collision().updateCircle(handle, x, y, radius);
}

} // namespace Engine::Collision
```

**Files to update:** `Asteroid.cpp`, `UFO.cpp`, `Ship.cpp`, `Bullet.cpp` ✓ Done

---

### Phase 4 — `Engine.h` aggregator

Once all subsystem facades exist, introduce the single aggregating header and
update games to include it.

---

### Phase 5 — Hide `Services` (future)

Move `Services.h` out of the public include path so the compiler enforces the
convention. This is a larger CMake restructure (separating engine public vs
private headers) and should be done deliberately, not alongside other work.

---

### Phase 6 — Doxygen API documentation (future)

Once the facade is stable, annotate facade headers with Doxygen comments and
generate a browsable HTML API reference. The facade is the right place for these
comments — it is the public contract for what games can do; internals are
implementation detail and don't need annotation.

**Comment style:**

```cpp
namespace Engine::Audio {

/// Play a synthesized tone.
/// @param frequencyHz  Pitch in Hz (e.g. 440 = A4).
/// @param durationSec  Duration of the tone in seconds.
/// @param amplitude    Volume in the range 0.0–1.0. Defaults to 0.4.
inline void playTone(float frequencyHz, float durationSec, float amplitude = 0.4f);

} // namespace Engine::Audio
```

**Scope:**
- Annotate facade headers only — `engine/facade/*.h` and `engine/Engine.h`
- Do not annotate engine internals (`Services`, `AudioManager`, etc.)
- Annotate shared types games construct directly (`ParticleEmitParams`,
  `ColliderDesc`, `ColliderHandle`)

**CI integration:**

Doxygen can run in CI on merge to main and publish to GitHub Pages, keeping the
reference always current. Add to the build pipeline after the facade is
feature-complete.

**Discipline:**

Doxygen comments must be updated in the same PR as the code change that
necessitates them — stale parameter docs are worse than none. Treat them the
same as the `docs/` markdown files.

---

## Migration strategy

- Introduce each facade on its own branch alongside the work that touches that
  subsystem
- Don't migrate all call sites speculatively — update them as files are
  naturally opened for other reasons
- The `[[deprecated]]` attribute can be used on old patterns during transition
  if needed, though in a monorepo a build error is equally effective

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

// Particles
Engine::Particles::emit(params);

// Collision
Engine::Collision::add(desc, callback);
Engine::Collision::remove(handle);
Engine::Collision::updateCircle(handle, x, y, radius);
```
