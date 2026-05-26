#pragma once

static constexpr int TILE     = 16;
static constexpr int SCALE    = 2;
static constexpr int MAP_COLS = 28;
static constexpr int MAP_ROWS = 31;
static constexpr int WIN_W    = MAP_COLS * TILE * SCALE; // 896
static constexpr int WIN_H    = MAP_ROWS * TILE * SCALE; // 992

static constexpr int kLayerWalls = 0;
static constexpr int kLayerDots  = 1;
