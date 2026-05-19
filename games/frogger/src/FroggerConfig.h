#pragma once

inline constexpr int COLS     = 13;
inline constexpr int ROWS     = 14;
inline constexpr int TILE_SRC = 16;
inline constexpr int SCALE    = 3;
inline constexpr int TILE     = TILE_SRC * SCALE;  // 48px on screen
inline constexpr int W        = COLS * TILE;        // 624
inline constexpr int H        = ROWS * TILE;        // 672

inline constexpr int SHEET_COLS = 8;
inline constexpr int SHEET_ROWS = 16;

inline constexpr int HOME_ROW        = 0;
inline constexpr int RIVER_FIRST_ROW = 1;
inline constexpr int RIVER_LAST_ROW  = 5;
inline constexpr int MEDIAN_ROW      = 6;
inline constexpr int ROAD_FIRST_ROW  = 7;
inline constexpr int ROAD_LAST_ROW   = 11;

inline constexpr int HOME_SLOTS[]    = { 1, 3, 6, 9, 11 };
inline constexpr int HOME_SLOT_COUNT = 5;
