# Sprite Rendering

## Overview

Sprite rendering is built from four layered concepts: a `Texture` that owns GPU memory, a `SpriteSheet` that divides it into a grid, an `AnimClip` that defines a named sequence of frames, and a `SpriteAnimator` that advances through clips over time. The renderer draws the same unit quad every frame — the only thing that changes per draw call is a four-float UV region uniform.

---

## Texture — Loading Pixels onto the GPU

A PNG on disk is just a grid of bytes. The GPU has its own memory (VRAM) and can't read from disk directly. `Texture` does a two-step process: load into RAM, then upload to VRAM.

**Step 1 — Load from disk:**
```cpp
unsigned char* data = stbi_load(path.c_str(), &m_width, &m_height, &channels, STBI_rgb_alpha);
```
`stbi_load` decodes the PNG into raw bytes. `STBI_rgb_alpha` forces 4 channels (RGBA) regardless of the source format. A 128×32 sheet produces 128 × 32 × 4 = 16,384 bytes.

**Step 2 — Upload to GPU:**
```cpp
glGenTextures(1, &m_id);
glBindTexture(GL_TEXTURE_2D, m_id);
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
stbi_image_free(data);
```
`glGenTextures` reserves an integer handle. `glTexImage2D` pushes the bytes to VRAM. After this call the GPU has its own copy and the CPU-side memory is freed.

**Filter parameters:**
```cpp
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
```
`GL_NEAREST` picks the closest pixel when scaling with no blending. `GL_LINEAR` would average neighbors and produce a blurry result — wrong for pixel art.

At draw time, `tex.bind(0)` points OpenGL at the texture on the GPU. The pixel data doesn't move.

---

## UV Coordinates

OpenGL uses a normalized 0–1 coordinate space for textures called UV space: U is horizontal, V is vertical, both 0.0–1.0 regardless of the texture's pixel dimensions.

The textured VAO in `Renderer2D` stores a unit quad where each vertex has both a position and a UV:

```cpp
float texVerts[] = {
    0.f, 0.f,  0.f, 1.f,   // pos bottom-left  → uv top-left    (V flipped)
    1.f, 0.f,  1.f, 1.f,   // pos bottom-right → uv top-right
    1.f, 1.f,  1.f, 0.f,   // pos top-right    → uv bottom-right
    0.f, 1.f,  0.f, 0.f,   // pos top-left     → uv bottom-left
};
```

Position (0,0) — bottom-left of the quad — maps to UV (0,1), and position (0,1) — top-left — maps to UV (0,0). This V-flip reconciles the PNG's top-down row order with OpenGL's bottom-up convention. This vertex data never changes.

What changes each draw call is the `u_uvRegion` uniform — a `vec4(u0, v0, u1, v1)` that tells the shader which sub-region of the texture to sample. The fragment shader remaps the base 0–1 UVs into that region:

```glsl
vec2 uv = vec2(
    u_uvRegion.x + v_uv.x * (u_uvRegion.z - u_uvRegion.x),
    u_uvRegion.y + v_uv.y * (u_uvRegion.w - u_uvRegion.y)
);
fragColor = texture(u_tex, uv);
```

This is linear interpolation. If `v_uv.x` is 0 the output is `u0`; if it's 1 the output is `u1`. One four-float uniform is all that's needed to select any frame — the geometry never changes.

---

## SpriteSheet — Dividing the Texture into a Grid

A sprite sheet is one texture holding multiple frames packed in a grid. You load it once and select sub-regions rather than swapping whole textures.

`SpriteSheet::getFrameUVs` computes the UV rectangle for any frame index:

```cpp
const int col = frameIndex % m_cols;
const int row = frameIndex / m_cols;
const float fw = 1.f / m_cols;
const float fh = 1.f / m_rows;
return { col * fw, row * fh, (col + 1) * fw, (row + 1) * fh };
```

Example for a 4×1 sheet (128×32px, four 32×32 frames):

| Frame | col | u0   | u1   | Pixels |
|-------|-----|------|------|--------|
| 0     | 0   | 0.00 | 0.25 | 0–31   |
| 1     | 1   | 0.25 | 0.50 | 32–63  |
| 2     | 2   | 0.50 | 0.75 | 64–95  |
| 3     | 3   | 0.75 | 1.00 | 96–127 |

The `%` and `/` math generalizes automatically — a 4×2 sheet computes frame 5 as col=1, row=1, giving u0=0.25, v0=0.5, u1=0.5, v1=1.0.

Multi-cell sprites (e.g. a 2×2 ship) use the overload `getFrameUVs(frameIndex, cellW, cellH)` which returns the UV rect spanning `cellW × cellH` cells starting at `frameIndex`.

---

## AnimClip and SpriteAnimator — Advancing Frames Over Time

An `AnimClip` defines a named sequence:

```cpp
Engine::AnimClip thrustClip;
thrustClip.frames        = { 4, 8 };      // sheet frame indices
thrustClip.frameDuration = 0.08f;
thrustClip.mode          = Engine::PlayMode::Loop;
thrustClip.wFrames       = 2;
thrustClip.hFrames       = 2;
```

`frames` is a list of sheet indices, not required to be linear. `{4, 5, 4, 5}` plays a two-frame clip, `{3, 2, 1, 0}` reverses it. `wFrames`/`hFrames` describe the cell size for multi-cell sprites.

`SpriteAnimator` answers "which frame should I show right now?" using an accumulator:

```cpp
m_accumulated += dt;
while (m_accumulated >= m_current->frameDuration) {
    m_accumulated -= m_current->frameDuration;
    m_frameIdx++;
}
```

The `while` (not `if`) handles the case where a large `dt` would span multiple frames. In `Loop` mode `m_frameIdx` wraps back to 0; in `OneShot` mode it clamps on the last frame.

`currentFrameUVs()` resolves the final UV rect:

```cpp
const int sheetFrame = m_current->frames[m_frameIdx];
return m_sheet->getFrameUVs(sheetFrame, m_current->wFrames, m_current->hFrames);
```

---

## End-to-End Trace

**onUpdate(dt)** — time bookkeeping only, no drawing:
```cpp
m_animator.update(dt);
```
Advances `m_accumulated`, maybe increments `m_frameIdx`. Nothing is drawn.

**onRender()** — read the result and draw:
```cpp
const Engine::UVRect uvs = m_animator.currentFrameUVs();
// e.g. m_frameIdx == 1 on a 4-frame sheet → {0.25, 0.0, 0.50, 1.0}

renderer.drawTexturedRect(x, y, 128.f, 128.f,
                          sheet.texture(),
                          uvs.u0, uvs.v0, uvs.u1, uvs.v1);
```

Inside `drawTexturedRect`:
1. Model matrix: translate to (x, y), scale by (128, 128) — unit quad becomes a 128×128 pixel square
2. `tex.bind(0)` — points OpenGL at the sprite sheet texture
3. Shader receives `u_mvp`, `u_uvRegion = (0.25, 0.0, 0.50, 1.0)`, `u_tex = 0`
4. GPU draws 6 indices (two triangles); each fragment's base UV is remapped into the 0.25–0.50 horizontal band

---

## Mental Model

```
PNG on disk
  → stbi_load: raw bytes in RAM
    → glTexImage2D: pixels on GPU (Texture owns the handle)
      → SpriteSheet: divides texture into a grid via UV math
        → AnimClip: named sequence of frame indices + timing
          → SpriteAnimator: advances frame index as time passes
            → currentFrameUVs(): returns {u0, v0, u1, v1} for the active frame
              → drawTexturedRect: sets u_uvRegion uniform, draws unit quad
                → fragment shader: remaps base UVs, samples texture
                  → color on screen
```

The geometry never changes. It's always the same unit quad, the same two triangles. What changes each frame is one four-float uniform: `u_uvRegion`. The GPU does the rest.
