 Here's a thorough walkthrough of how sprite rendering works end-to-end, using the actual code as
  examples:

  ---
  1. What a Texture Is and How It Gets onto the GPU

  A PNG on disk is just a grid of pixels. The GPU can't read your hard drive — it has its own memory
  (VRAM). Texture.cpp does a two-step process: load into RAM, then upload to VRAM.

  Step 1 — Load from disk:
  unsigned char* data = stbi_load(path.c_str(), &m_width, &m_height, &channels, STBI_rgb_alpha);
  stbi_load decodes the PNG and returns raw bytes. STBI_rgb_alpha forces 4 channels (RGBA) regardless
  of what the file stores. The test sheet is 128×32 pixels, so data points to 128 × 32 × 4 = 16,384
  bytes.

  Step 2 — Upload to GPU:
  glGenTextures(1, &m_id);
  glBindTexture(GL_TEXTURE_2D, m_id);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  glGenTextures reserves an integer handle. glTexImage2D pushes all the bytes to VRAM. After this
  call, the GPU has its own copy and the CPU-side memory is freed: stbi_image_free(data).

  The filter parameters matter:
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  GL_NEAREST means "pick the closest pixel when scaling, no blending." Using GL_LINEAR instead would
  average neighboring pixels and produce a blurry look — wrong for pixel art.

  Later, when drawing, tex.bind(0) just points OpenGL's finger at this texture on the GPU. The actual
  pixel data doesn't move.

  ---
  2. What UV Coordinates Are

  OpenGL works in a normalized 0–1 coordinate space for textures called UV space: U is horizontal, V
  is vertical, both 0.0–1.0 regardless of the texture's actual pixel dimensions.

  The textured VAO in Renderer2D.cpp stores a unit quad where every vertex has both a position and a
  UV:

  float texVerts[] = {
      0.f, 0.f,  0.f, 1.f,   // pos bottom-left  → uv top-left    (V flipped)
      1.f, 0.f,  1.f, 1.f,   // pos bottom-right → uv top-right
      1.f, 1.f,  1.f, 0.f,   // pos top-right    → uv bottom-right
      0.f, 1.f,  0.f, 0.f,   // pos top-left     → uv bottom-left
  };
  Notice position (0,0) — the bottom-left of the quad — is paired with UV (0,1), and position (0,1) —
  the top-left — is paired with UV (0,0). That V-flip reconciles the PNG's top-down row order with
  OpenGL's bottom-up convention. This vertex data never changes.

  What does change each draw call is the u_uvRegion uniform — a vec4(u0, v0, u1, v1) that tells the
  shader which sub-region of the texture to use. The fragment shader remaps the base 0–1 UVs into that
   region:
  vec2 uv = vec2(
      u_uvRegion.x + v_uv.x * (u_uvRegion.z - u_uvRegion.x),  // remap U into u0→u1
      u_uvRegion.y + v_uv.y * (u_uvRegion.w - u_uvRegion.y)   // remap V into v0→v1
  );
  fragColor = texture(u_tex, uv);
  This is just a linear interpolation. If v_uv.x is 0 the output is u0; if it's 1 the output is u1.
  The geometry never changes — one four-float uniform is all that's needed to select any frame.

  ---
  3. How a Sprite Sheet Works

  A sprite sheet is one texture holding multiple frames packed side by side. You load it once and
  select sub-regions instead of swapping whole textures. The test asset is 128×32 pixels — four 32×32
  frames in a single row, each a different solid color.

  SpriteSheet::getFrameUVs computes the UV rectangle for any frame index:
  const int col = frameIndex % m_cols;   // which column
  const int row = frameIndex / m_cols;   // which row
  const float fw = 1.f / 4.f;           // = 0.25 (each frame is 1/4 of the width)
  const float fh = 1.f / 1.f;           // = 1.0  (only one row)

  All four frames concretely:

  ┌───────┬─────┬──────┬──────┬─────────────────┐
  │ Frame │ col │  u0  │  u1  │  Pixel columns  │
  ├───────┼─────┼──────┼──────┼─────────────────┤
  │ 0     │ 0   │ 0.00 │ 0.25 │ 0–31 (red)      │
  ├───────┼─────┼──────┼──────┼─────────────────┤
  │ 1     │ 1   │ 0.25 │ 0.50 │ 32–63 (green)   │
  ├───────┼─────┼──────┼──────┼─────────────────┤
  │ 2     │ 2   │ 0.50 │ 0.75 │ 64–95 (blue)    │
  ├───────┼─────┼──────┼──────┼─────────────────┤
  │ 3     │ 3   │ 0.75 │ 1.00 │ 96–127 (yellow) │
  └───────┴─────┴──────┴──────┴─────────────────┘

  The % and / math generalizes automatically — a 4×2 sheet with 8 frames would compute frame 5 as
  col=1, row=1 and give u0=0.25, v0=0.5, u1=0.5, v1=1.0.

  ---
  4. How the Animator Advances Frames Over Time

  The animator's only job is answering "which frame should I show right now?" It uses an accumulator
  pattern:

  m_accumulated += dt;
  while (m_accumulated >= m_current->frameDuration) {
      m_accumulated -= m_current->frameDuration;
      m_frameIdx++;
      ...
  }

  Each tick, elapsed time accumulates. When it exceeds one frame's duration (0.2s), exactly 0.2s is
  subtracted and the frame index advances. The while loop (not if) handles the case where a large dt
  would span multiple frames at once. At 60 FPS, dt ≈ 0.0167s, so the frame advances roughly every 12
  ticks.

  In Loop mode m_frameIdx wraps back to 0. In OneShot mode it clamps on the last frame and drains the
  accumulator.

  The indirection in currentFrameUVs is worth noting:
  const int sheetFrame = m_current->frames[m_frameIdx];
  return m_sheet->getFrameUVs(sheetFrame);
  The clip's frames array is a list of sheet indices — {0, 1, 2, 3} for the walk cycle, but it could
  be {4, 5, 4, 5} for a different 2-frame clip on the same sheet, or {3, 2, 1, 0} for a reversed
  cycle. The animator isn't hardcoded to linear sequences.

  ---
  5. Tracing a Single Frame End-to-End

  onUpdate(dt) — pure time bookkeeping, no drawing:
  m_animator->update(dt);
  This adds dt to m_accumulated and maybe increments m_frameIdx. Nothing else happens.

  onRender() — reading the result and drawing it:
  const Engine::UVRect uvs = m_animator->currentFrameUVs();
  // Say m_frameIdx == 1: returns {0.25, 0.0, 0.50, 1.0}

  m_renderer.drawTexturedRect(x, y, 128.f, 128.f,
                              m_sheet->texture(),
                              uvs.u0, uvs.v0, uvs.u1, uvs.v1);

  Inside drawTexturedRect:
  1. Model matrix: translate to (336, 236), scale by (128, 128) — the unit quad becomes a 128×128
  pixel square at the center of the window
  2. tex.bind(0) — points OpenGL at the sprite sheet texture
  3. Shader gets u_mvp, u_uvRegion = (0.25, 0.0, 0.50, 1.0), u_tex = 0
  4. The GPU draws 6 indices (two triangles), each fragment's base UV remapped into the 0.25–0.50
  horizontal band — the green frame

  ---
  The Full Mental Model

  PNG on disk
    → stbi_load: raw bytes in RAM
      → glTexImage2D: pixels on GPU (Texture owns the handle)
        → SpriteSheet: divides texture into a grid via UV math
          → AnimClip: sequence of frame indices + timing
            → SpriteAnimator: advances frame index as time passes
              → currentFrameUVs(): returns {u0,v0,u1,v1} for active frame
                → drawTexturedRect: sets u_uvRegion uniform, draws unit quad
                  → fragment shader: remaps base UVs, samples texture
                    → color on screen

  The key insight: the geometry never changes. It's always the same unit quad, the same two triangles.
   What changes each frame is one four-float uniform: u_uvRegion. The GPU does the rest.

