# Application and Game Loop

## Overview

`Engine::Application` is the base class every game inherits from. It owns the window, the collision world, and the fixed-rate tick loop. Games subclass it and override lifecycle hooks to plug in their logic. The engine calls those hooks in a defined order each tick — games don't drive the loop themselves.

---

## Creating a Game

Subclass `Application`, pass a title and window dimensions to the base constructor, and override whichever hooks you need:

```cpp
class AsteroidsGame : public Engine::Application {
public:
    AsteroidsGame() : Engine::Application("Asteroids", 800, 800) {
        // one-time setup — load assets, spawn initial objects
    }

protected:
    void preStep(float dt)  override; // move objects, sync colliders
    void onUpdate(float dt) override; // game logic, collision response
    void onRender()         override; // draw everything
};
```

`main.cpp` just instantiates the game and calls `run()`:

```cpp
int main() {
    AsteroidsGame game;
    game.run();
}
```

---

## Lifecycle Hooks

| Hook | When it runs | Typical use |
|---|---|---|
| `onInit()` | Once, before the loop starts | Late setup that needs the window ready |
| `preStep(dt)` | Every tick, before collision | Move objects, sync collider positions |
| `onUpdate(dt)` | Every tick, after collision | Game logic, collision response, spawning |
| `onRender()` | Every frame, after all ticks | Draw calls only — no state mutation |
| `onShutdown()` | Once, after the loop exits | Cleanup |

All hooks have empty default implementations, so you only override what you need.

---

## Fixed-Rate Tick Loop

The engine runs game logic at a fixed rate of **60 Hz** regardless of how fast frames render. This decouples physics/collision stability from display performance.

```
each frame:
  frameTime = now - prevTime  (clamped to 0.25s max)
  accum    += frameTime

  while accum >= 1/60:
      Input::update()
      preStep(1/60)
      CollisionWorld::step()
      onUpdate(1/60)
      accum -= 1/60

  onRender()
  swapBuffers()
```

The `0.25s` clamp on `frameTime` prevents the **spiral of death**: if a frame takes too long (debugger pause, system hiccup), the engine doesn't try to catch up with an unbounded burst of ticks. It simply accepts the lost time and moves on.

`dt` passed to `preStep` and `onUpdate` is always exactly `1/60 ≈ 0.01667s`. Never use wall-clock time for game logic — always use `dt`.

`onRender` runs once per frame regardless of how many ticks fired. If the machine is fast enough, it may render multiple frames between ticks (interpolation not currently implemented). If it's slow, multiple ticks may fire per frame.

---

## Tick Order Detail

Within each tick, the order matters for correctness:

```
Input::update()         — snapshot key state for this tick
preStep(dt)             — move all objects, sync collider positions to current locations
CollisionWorld::step()  — test overlaps against current positions, fire callbacks
onUpdate(dt)            — read collision results, spawn/erase objects, handle input
```

`preStep` exists specifically so collision sees current-tick positions rather than last-tick positions. See `docs/engine/collision-system.md` for why this ordering matters.

---

## Subsystem Initialization

The `Application` constructor sets up all engine subsystems before `onInit` or any game constructor body runs. By the time your game's constructor body executes, audio, particles, and collision are all ready — game objects can call `Engine::Collision::add()`, `Engine::Audio::playTone()`, etc. immediately, including from objects constructed in the game's constructor body.

Game code accesses subsystems through the facade (`<engine/Engine.h>`), not through engine internals. See `docs/engine/engine-facade.md` for the full API.

---

## Quitting

Call `quit()` from anywhere in your game to exit the loop cleanly after the current frame:

```cpp
if (Engine::Input::isKeyPressed(GLFW_KEY_Q))
    quit();
```
