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

## beginScene

Call once per frame before any draw calls:

```cpp
m_renderer.beginScene(W, H);
```

This clears the screen to black and rebuilds the projection matrix for the given dimensions. It does not need to be called between ticks — only once at the start of `onRender`.

---

## drawRect — Colored Rectangle

```cpp
renderer.drawRect(float x, float y, float w, float h, const glm::vec4& color);
```

Draws a solid-colored rectangle. `(x, y)` is the **top-left corner**. `color` is RGBA in 0–1 range.

```cpp
// White 32×32 square at (100, 200)
renderer.drawRect(100.f, 200.f, 32.f, 32.f, { 1.f, 1.f, 1.f, 1.f });
```

No rotation support. Uses a simple vertex + fragment shader with a `u_color` uniform.

---

## drawTexturedRect — Sprite Rectangle

```cpp
renderer.drawTexturedRect(float x, float y, float w, float h,
                          const Texture& tex,
                          float u0 = 0.f, float v0 = 0.f,
                          float u1 = 1.f, float v1 = 1.f,
                          float angle = 0.f);
```

Draws a textured rectangle. `(x, y)` is the **top-left corner** before rotation. `u0/v0/u1/v1` select a UV sub-region of the texture (see `docs/engine/sprite-rendering.md`). `angle` rotates the sprite around its center in radians.

```cpp
// Draw a sprite frame at (50, 50), 64×64 pixels, rotated 45°
renderer.drawTexturedRect(50.f, 50.f, 64.f, 64.f,
                          sheet.texture(),
                          uv.u0, uv.v0, uv.u1, uv.v1,
                          glm::radians(45.f));
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
void AsteroidsGame::onRender() {
    m_renderer.beginScene(W, H);        // clear + set projection

    for (const auto& a : m_asteroids)
        a->render(m_renderer, *m_sheet);

    m_ship->render(m_renderer);

    for (const auto& b : m_bullets)
        b->render(m_renderer);
    // no endScene / present call — Application::run() calls swapBuffers
}
```

`swapBuffers` is called by `Application::run()` after `onRender` returns. Don't call it yourself.
