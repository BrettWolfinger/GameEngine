# Input

## Overview

`Engine::Input` is a static class that wraps GLFW keyboard state into two simple queries: whether a key is currently held, and whether a key was pressed for the first time this tick. It's updated once per tick before `preStep` runs, so all reads within a tick see consistent state.

---

## API

```cpp
Engine::Input::isKeyDown(int key)    // true every tick the key is held
Engine::Input::isKeyPressed(int key) // true only on the first tick the key goes down
```

Key constants come from GLFW: `GLFW_KEY_SPACE`, `GLFW_KEY_LEFT`, `GLFW_KEY_W`, etc.

**Choosing between them:**

| Use case | Function |
|---|---|
| Continuous action (thrust, rotate, hold to charge) | `isKeyDown` |
| One-shot action (fire, quit, toggle, menu select) | `isKeyPressed` |

Using `isKeyDown` for a one-shot action (like firing) means the action repeats every tick the key is held — 60 times per second. Use `isKeyPressed` when you want exactly one trigger per physical key press.

---

## How It Works

Input maintains four arrays of `GLFW_KEY_LAST + 1` booleans:

| Array | Purpose |
|---|---|
| `s_pending` | Written by GLFW's key callback as events arrive |
| `s_justPressed` | Snapshot of `s_pending` taken at the start of each tick |
| `s_curr` | Current raw key state polled from GLFW this tick |
| `s_prev` | Raw key state from the previous tick (unused externally) |

**Each tick**, `Input::update()` runs before any game code:

```cpp
void Input::update() {
    s_justPressed = s_pending;  // snapshot callback-driven presses
    s_pending     = {};         // clear for next tick
    s_prev = s_curr;
    for (int k = 0; k <= GLFW_KEY_LAST; ++k)
        s_curr[k] = glfwGetKey(s_window, k) == GLFW_PRESS;
}
```

`isKeyDown` reads `s_curr` — the polled state, true for as long as the key is held.

`isKeyPressed` reads `s_justPressed` — the callback snapshot, true only for the one tick the OS delivered the key-down event.

**Why two mechanisms?** Polling (`glfwGetKey`) is reliable for held state but can miss brief presses between ticks if the key is released before `update()` runs. The callback path (`s_pending`) captures the event the moment the OS delivers it, so no press is dropped even at 60 Hz.

---

## Usage Examples

```cpp
// Ship rotation — held key, continuous effect
if (Engine::Input::isKeyDown(GLFW_KEY_LEFT))
    m_angle -= ROTATE_SPEED * dt;

// Quit — one-shot, fires exactly once per press
if (Engine::Input::isKeyPressed(GLFW_KEY_Q))
    quit();

// Shooting — gated by fire cooldown, but uses isKeyDown so holding fires repeatedly
// (the cooldown timer is what limits the rate, not isKeyPressed)
if (m_fireTimer <= 0.f && Engine::Input::isKeyDown(GLFW_KEY_SPACE))
    // fire
```
