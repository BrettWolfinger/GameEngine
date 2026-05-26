# Renderer2D

## Overview

`Engine::Renderer2D` draws axis-aligned rectangles in screen space — either a solid color or a textured sprite sub-region. It manages two independent OpenGL pipelines that share the same unit-quad geometry. All positioning and sizing is done via model matrix uniforms; the vertex data on the GPU never changes.

---

## Coordinate System

The renderer uses a **Y-down screen-space** coordinate system:

- `(0, 0)` is the **top-left** of the window
- X increases to the right
- Y increases **downward**
- `(width, height)` is the bottom-right

This matches how most 2D games and image formats think about coordinates. `beginScene` sets up a matching orthographic projection:

```cpp
m_proj = glm::ortho(0.f, (float)width, (float)height, 0.f, -1.f, 1.f);
//                  left  right         bottom          top
```

All draw call positions are in these screen-space pixel units.

---

## Render Layers

Every draw call is tagged with a **layer** integer (default `0`). Draws accumulate into per-layer buckets and are flushed in ascending layer order when `endScene()` is called — higher layers always render on top of lower layers, regardless of call order within `onRender`.

```cpp
inline constexpr int kLayerBackground = 0;
inline constexpr int kLayerWorld      = 1;
inline constexpr int kLayerEffects    = 2;
inline constexpr int kLayerHUD        = 3;

// In onRender():
renderer.drawTexturedRect(..., kLayerBackground); // maze bg drawn first
renderer.drawTexturedRect(..., kLayerWorld);       // pacman and ghosts on top
renderer.drawRect(...,         kLayerHUD);         // score always above everything
```

Define layer constants in `GameConstants.h`. The renderer supports up to **16 layers** (0–15). Layer 15 (`kParticleLayer`) is reserved for the engine's particle system — do not use it in game code.

**Two flush passes per frame** — Application calls `endScene()` automatically:
1. After `onRender()` + particle rendering — flushes the world.
2. After `onOverlayRender()` — flushes the overlay. Overlay draws always appear above world draws and particles regardless of which layer numbers are used.

---

## beginScene

Call once per frame before any draw calls:

```cpp
m_renderer.beginScene(W, H);
```

Clears the screen to black, rebuilds the projection matrix for the given dimensions, and clears all layer buckets. Call only once at the start of `onRender` — not between ticks.

---

## endScene

Flushes all layer buckets in ascending order (0 → 15), then clears them. Called automatically by Application — do not call it yourself.

---

## drawRect — Colored Rectangle

```cpp
renderer.drawRect(float x, float y, float w, float h,
                  const glm::vec4& color, int layer = 0);
```

Queues a solid-colored rectangle into the given layer. `(x, y)` is the **top-left corner**. `color` is RGBA in 0–1 range.

```cpp
// White 32×32 square at (100, 200) on the HUD layer
renderer.drawRect(100.f, 200.f, 32.f, 32.f, { 1.f, 1.f, 1.f, 1.f }, kLayerHUD);
```

---

## drawTexturedRect — Sprite Rectangle

```cpp
renderer.drawTexturedRect(float x, float y, float w, float h,
                          const Texture& tex,
                          float u0 = 0.f, float v0 = 0.f,
                          float u1 = 1.f, float v1 = 1.f,
                          float angle = 0.f,
                          const glm::vec4& tint = {1,1,1,1},
                          int layer = 0);
```

Queues a textured rectangle into the given layer. `(x, y)` is the **top-left corner** before rotation. `u0/v0/u1/v1` select a UV sub-region of the texture (see `docs/engine/sprite-rendering.md`). `angle` rotates the sprite around its center in radians. `tint` multiplies the sampled color.

```cpp
// Sprite frame at (50, 50), 64×64 pixels, rotated 45°, on the world layer
renderer.drawTexturedRect(50.f, 50.f, 64.f, 64.f,
                          sheet.texture(),
                          uv.u0, uv.v0, uv.u1, uv.v1,
                          glm::radians(45.f),
                          {1,1,1,1},
                          kLayerWorld);
```

**Rotation** is applied around the sprite's center. The model matrix translates to center, rotates, translates back, then scales — so `(x, y)` stays the intended top-left of the unrotated bounding box.

**Alpha discard:** the fragment shader discards pixels with alpha < 0.1, which gives clean edges on sprite sheets with transparent backgrounds without requiring blending setup.

---

## How Both Pipelines Work

Both `drawRect` and `drawTexturedRect` use the same unit quad stored once in GPU memory:

```
(0,0) ──── (1,0)
  │           │
(0,1) ──── (1,1)
```

Per draw call, a model matrix scales and translates it to the desired position and size:

```cpp
glm::mat4 model = glm::translate(glm::mat4(1.f), glm::vec3(x, y, 0.f));
model = glm::scale(model, glm::vec3(w, h, 1.f));
```

The `u_mvp = proj * model` uniform is all that moves the quad. No per-draw VBO uploads.

For the textured pipeline, a second uniform `u_uvRegion = vec4(u0, v0, u1, v1)` remaps the base 0–1 UVs into the requested sub-region in the fragment shader. See `docs/engine/sprite-rendering.md` for the UV remapping math.

---

## Typical onRender Pattern

```cpp
void MyGame::onRender() {
    m_renderer.beginScene(W, H);   // clear screen + set projection + clear layer buckets

    // Queue draws into layers — order of calls within a layer doesn't matter
    // for cross-layer ordering, only for same-layer ordering.
    m_renderer.drawTexturedRect(..., kLayerBackground);
    for (const auto& e : m_enemies)
        e->render(m_renderer, kLayerWorld);
    m_player.render(m_renderer, kLayerWorld);

    // Application calls endScene() and swapBuffers() — do not call them here.
}
```

`endScene()` and `swapBuffers()` are both called by `Application::run()` — don't call them yourself.

**`getRenderer()` is required** for the flush to fire. Override it in every game that uses `Renderer2D`:

```cpp
Engine::Renderer2D* getRenderer() override { return &m_renderer; }
```
