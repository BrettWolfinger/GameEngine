#pragma once

// NES tile size (16×16 source pixels)
static constexpr int TILE  = 16;
// Render scale — NES native is 256×240, scale 3 gives 768×720
static constexpr int SCALE = 3;

// Visible screen in tiles (NES: 16 cols × 15 rows)
static constexpr int SCREEN_COLS = 16;
static constexpr int SCREEN_ROWS = 15;

static constexpr int WIN_W = SCREEN_COLS * TILE * SCALE; // 768
static constexpr int WIN_H = SCREEN_ROWS * TILE * SCALE; // 720

// Mario sprite frame dimensions (native pixels)
static constexpr int MARIO_FRAME_W = 32;
static constexpr int MARIO_FRAME_H = 16;
