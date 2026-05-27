#pragma once

static constexpr int TILE     = 16;
static constexpr int SCALE    = 2;
static constexpr int MAP_COLS = 28;
static constexpr int MAP_ROWS = 31;
static constexpr int WIN_W    = MAP_COLS * TILE * SCALE; // 896
static constexpr int WIN_H    = MAP_ROWS * TILE * SCALE; // 992

static constexpr int kLayerWalls  = 0;
static constexpr int kLayerDots   = 1;
static constexpr int kLayerGhosts = 2;
static constexpr int kLayerPacman = 3;
static constexpr int kLayerHUD    = 4;

// Ghost spritesheet layout (32px native cells, 4 cols x 11 rows)
// Rows 0-3: Blinky, Inky, Pinky, Clyde (columns = animation frames)
// Rows 4-7: alternate ghost color variants (unused)
// Row 8:    frightened (blue)
// Rows 9-10: unused
static constexpr int kGhostSheetCols     = 4;
static constexpr int kGhostSheetRows     = 11;
static constexpr int kGhostFrightenedRow = 8;

// Dot scoring
static constexpr int kScoreDot    = 10;
static constexpr int kScorePellet = 50;

// Ghost eating scores (doubles per ghost per pellet: 200, 400, 800, 1600)
static constexpr int kGhostScoreBase = 200;

// Frightened mode duration (seconds, Level 1)
static constexpr float kFrightenedDuration = 7.f;

// Lives
static constexpr int   kStartLives  = 3;

// Death sequence: pause between animation end and respawn (seconds)
static constexpr float kDeathPause  = 1.0f;

// Level-clear freeze before restarting (seconds)
static constexpr float kLevelClearPause = 3.0f;
