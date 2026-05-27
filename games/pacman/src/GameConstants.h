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
// Row = ghost type (0=Blinky,1=Pinky,2=Inky,3=Clyde)
// Within each row: right(0), left(1), up(2), down(3)
static constexpr int kGhostSheetCols = 4;
static constexpr int kGhostSheetRows = 11;

// Dot scoring
static constexpr int kScoreDot    = 10;
static constexpr int kScorePellet = 50;
