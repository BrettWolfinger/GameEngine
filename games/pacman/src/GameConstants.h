#pragma once
#include <iterator> // std::size

static constexpr int TILE     = 16;
static constexpr int SCALE    = 2;
static constexpr int MAP_COLS  = 28;
static constexpr int MAP_ROWS  = 31;
static constexpr int HUD_H     = TILE * 3;               // 48px — HUD strip above the maze
static constexpr int WIN_W     = MAP_COLS * TILE * SCALE; // 896
static constexpr int WIN_H     = MAP_ROWS * TILE * SCALE + HUD_H; // 1040

static constexpr int kLayerWalls  = 0;
static constexpr int kLayerDots   = 1;
static constexpr int kLayerGhosts = 2;
static constexpr int kLayerPacman = 3;
static constexpr int kLayerHUD    = 4;

// Ghost spritesheet layout (32px native cells, 4 cols x 11 rows)
// Rows 0-3: Blinky, Inky, Pinky, Clyde (columns = animation frames)
// Rows 4-7: alternate ghost color variants (unused)
// Row 8:    frightened (blue)
// Row 9:    frightened flash (white — interleaved with row 8 for flash clip)
// Row 10:   unused
static constexpr int kGhostSheetCols     = 4;
static constexpr int kGhostSheetRows     = 11;
static constexpr int kGhostFrightenedRow = 8;
static constexpr int kGhostFlashRow      = 9;

// Face spritesheet layout (PacManFaces.png, 16px native cells, 8 cols x 2 rows)
// Row 0, frames 0-3: Blinky, Pinky, Inky, Clyde faces (matching GhostType enum)
// Remaining cells unused.
static constexpr int kFaceSheetCols      = 8;
static constexpr int kFaceSheetRows      = 2;

// Ghost house geometry
static constexpr int kGhostHouseRow     = 12;  // door row; row < this = outside
static constexpr int kGhostHouseExitCol = 13;  // Leaving mode target column
static constexpr int kGhostHouseExitRow = 11;  // Leaving mode target row

// Dot-counter ghost release thresholds (dots eaten before ghost is released)
static constexpr int kReleaseDotsInky   = 30;
static constexpr int kReleaseDotsClyde  = 60;

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

// Level 1 scatter/chase schedule (seconds): Scatter 7, Chase 20, Scatter 7,
// Chase 20, Scatter 5, Chase 20, Scatter 5, then Chase permanently.
static constexpr float kModeSchedule[] = { 7.f, 20.f, 7.f, 20.f, 5.f, 20.f, 5.f };
static constexpr int   kModeCount      = static_cast<int>(std::size(kModeSchedule));
