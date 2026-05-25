# Devlog

Chronological record of development decisions and progress. Captures the *why*
behind the work — context that doesn't fit in commit messages or architecture docs.

Oldest entries at the top.

---

## Foundation

The engine launched with the core systems needed to get anything on screen:
`Application` (fixed-timestep game loop), `Window` (GLFW wrapper), `Renderer2D`
(solid-color quad drawing), `Shader`, and `AudioManager`. These were the minimum
to build Pong.

The fixed-timestep loop was a deliberate early decision — update logic runs at a
fixed 60Hz regardless of frame rate, with rendering uncapped. This keeps physics
and input deterministic across machines and avoids the class of bugs where game
speed scales with frame rate.

---

## Pong

### Input system

Pong's mode select exposed a bug: key presses were being dropped when the
fixed-timestep loop skipped an `onUpdate`. The fix introduced a two-buffer
callback pattern (`s_pending` → `s_justPressed`) and moved `Input::update()`
inside the fixed-timestep inner loop, making "just pressed" detection reliable.

### SegmentFont

Pong needed 7-segment style digits for scores and short labels. Rather than
leave it as a game-specific helper it was promoted to an engine subsystem
(`Engine::SegmentFont`) so any future game could use it without reimplementing
it. This established an early pattern: if something is general-purpose and
display-facing, it belongs in the engine.

### Dev keys

As gameplay complexity grew, testing specific states (win screen, near-win score)
by playing through became tedious. A compile-time gated system (`ENABLE_DEV_KEYS`,
automatically on for non-Release builds) was added so games could expose F-key
shortcuts without shipping them.

---

## Breakout

### PixelFont

Breakout needed more text than Pong: titles, prompts, game-over messages. The
7-segment style looked poor for words, so a 5×7 bitmask pixel font was built at
the engine level. The convention that emerged: `SegmentFont` for numeric displays
(scores, lives, countdowns), `PixelFont` for all word labels. Pong was
intentionally left on `SegmentFont`-only to show the engine's progression across
games.

### SaveData

Breakout introduced the first need for state that survives between sessions (high
score). A thin key/value save slot (`Engine::SaveData`) backed by a plain-text
file was promoted to the engine so the pattern is available to every future game
without duplicating file I/O plumbing.

---

## Frogger

### Sprite rendering

Frogger was the first game to use sprites, which drove a significant expansion of
the renderer. `Texture` was added as an RAII wrapper around an OpenGL texture
object — loads any image via stb_image, uploads as GL_RGBA8 with nearest-neighbor
filtering for pixel-art crispness. `SpriteSheet` wraps a texture and divides it
into a uniform cell grid; `getFrameUVs` returns the UV rect for a single cell,
`getSpanUVs` spans multiple consecutive cells horizontally for wide sprites like
vehicles. `SpriteAnimator` drives frame-based animation from named clips
(`AnimClip`), each with a frame list, per-frame duration, and a `PlayMode` of
`Loop` or `OneShot`.

`Renderer2D` was extended to handle textured quads with UV region parameters and
optional rotation — a second VAO/VBO/shader pair handles textured draws
separately from solid-color ones. The fragment shader discards fragments below
alpha 0.1 for transparency. A V-flip bug was also caught and fixed here: the
engine's Y-down orthographic projection maps (0,0) to screen top-left, which must
map to V=0 (image top). The original vertex data had this inverted, flipping all
sprites upside-down.

### Gameplay

Frogger introduced the engine's first tile-based movement system — the frog hops
on a fixed grid rather than moving continuously. The game is split into two zones:
a road zone where vehicles must be avoided, and a river zone where the frog must
ride moving platforms (logs, turtles, crocodiles) to cross. Falling in the water
or being hit by a vehicle ends the life immediately.

The turtles and crocodiles added per-entity behavioral state that wasn't needed in
earlier games. Turtles periodically dive and become unsafe — the dive animation
is the only visual warning. Crocodiles open their mouths periodically; landing on
the head during this state is fatal, but the body and tail are always safe. A
skull marker briefly appears on the tile where the frog last died before the next
attempt begins. Filling all five home slots wins the round.

---

## Asteroids

### Collision system

Asteroids was the first game complex enough to need general collision detection.
`CollisionWorld` was added to the engine with `ColliderDesc` (circle and AABB
shapes), layer/mask bitmask filtering, and opaque `ColliderHandle` for registration
and removal. A `Services::collision()` service locator wired it into the
application. Collision callbacks fire per-pair each fixed tick, with the constraint
that game objects cannot be removed from containers during dispatch — side effects
must be deferred until after `step()` returns.

A `preStep` hook (`virtual void preStep(float dt)`) was added to `Application`,
called between `Input::update()` and `CollisionWorld::step()`. This lets games
sync collider positions before collision runs, so detections are against
current-frame positions rather than last-frame.

### UI and rendering additions

`Engine::Menu` — a reusable cursor-driven menu widget with Up/Down navigation,
Enter confirmation, and configurable colors, spacing, and cursor glyph — was
added as the first entry in a new `ui/` subsystem. Alpha blending (`GL_BLEND`)
was enabled in `Renderer2D::beginScene()`, and a tint parameter was added to
`drawTexturedRect` (backed by a `u_tint` uniform), with white as default so all
existing callers were unaffected.

### Config tables

Asteroids has three ship types, three asteroid sizes, and a UFO — each with
distinct physics, audio, particle, and scoring parameters. Rather than scatter
magic numbers through the game code, constexpr config tables were introduced
(`ShipConfig`, `AsteroidSizeConfig`, `UfoConfig`) indexed by type. All
game-specific behavior for a given entity derives from a single row in the
appropriate table.

### Game structure

Asteroids introduced the most structured screen flow of any game so far: title,
ship select, playing, and game over screens share state through a `GameContext`
struct passed by reference. The ship select screen lets the player choose between
four ships with meaningfully different handling profiles (Fighter, Scout, Gunship,
Racer), persisting the selection across replays.

The wave system starts with four large asteroids spawned at a safe distance from
the ship. Clearing a wave triggers a brief delay before the next spawns with more
asteroids. Large asteroids fragment into medium on destruction; medium fragment
into small. A UFO appears roughly 15 seconds into each session, fires at the
player, and respawns every 20 seconds. Player and UFO bullets use separate
collision layers so UFO fire can't destroy asteroids. Lives, score, and a
persistent high score round out the progression.

---

## Engine hardening (between Asteroids and next game)

### The problem

When `AudioManager` changed from static to instance methods, both Pong and
Breakout broke — they were calling `Engine::Services::audio().playTone(...)` and
`Engine::AudioManager::playTone(...)` directly. The root cause was that game code
was coupled to engine internals: `Services` is an internal registry, not a public
API. Any refactor of how the engine manages subsystems forces fixes across every
game file.

### Engine facade

A facade layer was introduced: free functions grouped in `Engine::Audio::`,
`Engine::Particles::`, and `Engine::Collision::` namespaces. Games call these
stable functions; the facade owns the mapping to the current internal
implementation. The facade headers are pure declarations — no inline
implementation, no internal includes — with `.cpp` files compiled as part of the
engine holding the actual wrappers. `Engine.h` aggregates all three facades into a
single include for game code.

Enforcement was added at the compiler level. An `ENGINE_INTERNAL` preprocessor
guard was added to `Services.h`, `AudioManager.h`, `ParticleSystem.h`, and
`CollisionWorld.h`; the engine CMake target defines it privately. Game code that
tries to include these headers directly gets a compile error with a message
pointing to `<engine/Engine.h>`. `Application` was refactored to pimpl so its
subsystem members (`AudioManager`, `ParticleSystem`, `CollisionWorld`) live in a
private `struct Impl` in `Application.cpp`, removing all internal includes from
the public header.

### Documentation

All public facade headers and shared game types were annotated with Doxygen `///`
comments. The engine facade roadmap document was converted into a stable
architecture reference at `docs/engine/engine-facade.md` — covering the design
rationale, what belongs in the facade vs. what doesn't, enforcement details, and
how to add new subsystems. Doxygen CI config and GitHub Pages publishing are
tracked separately in issue #68. READMEs were written for Asteroids and Frogger.
