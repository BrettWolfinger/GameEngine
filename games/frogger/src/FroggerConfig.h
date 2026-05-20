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
inline constexpr int HOME_FILLED_FRAME = 4;    // seated frog sprite: row 0 col 4 (0-indexed)

// ---------- Death feedback ----------
inline constexpr int   SKULL_FRAME            = 8;     // skull sprite: row 1 col 0 (0-indexed)
inline constexpr float DEATH_DISPLAY_DURATION = 1.5f;

// ---------- Game rules ----------
inline constexpr int LIVES_START = 3;

// ---------- Platforms (river) ----------

enum class PlatformType { Log, Turtle };

// frameFirst = left cap (log) or single tile (turtle)
// frameMid   = middle segment (log); same as frameFirst for non-segmented types
// frameLast  = right cap (log); same as frameFirst for non-segmented types
struct PlatformTypeInfo {
    int frameFirst;
    int frameMid;
    int frameLast;
};

inline constexpr PlatformTypeInfo PLATFORM_TYPE_INFO[] = {
    /* Log    */ { 24, 25, 26 },  // row 3 cols 0-2 (0-indexed)
    /* Turtle */ { 17, 17, 17 },  // row 2 col 1 (0-indexed)
};

struct RiverLaneConfig {
    int          row;
    int          direction;
    float        speed;
    PlatformType type;
    int          tileWidth;  // tiles wide; controls log length via extra middle segments
    int          count;
    float        spacing;
};

inline constexpr RiverLaneConfig RIVER_LANE_CONFIGS[] = {
    { 1,  1,  70.f, PlatformType::Log,    3, 3, 280.f },  // medium logs
    { 2, -1,  90.f, PlatformType::Turtle, 2, 3, 230.f },  // turtle pairs
    { 3,  1, 120.f, PlatformType::Log,    2, 2, 320.f },  // short logs (fast)
    { 4, -1,  60.f, PlatformType::Turtle, 2, 3, 220.f },  // turtle pairs
    { 5,  1,  80.f, PlatformType::Log,    4, 2, 350.f },  // long logs
};
inline constexpr int RIVER_LANE_COUNT = 5;

// ---------- Vehicles ----------

enum class VehicleType { Car, RaceCar, Truck };

struct VehicleTypeInfo {
    int spriteFrame;  // row-major frame index into the sprite sheet
    int tileWidth;    // width in tiles (Truck = 2, others = 1)
};

inline constexpr VehicleTypeInfo VEHICLE_TYPE_INFO[] = {
    /* Car     */ { 48, 1 },  // sheet row 6, col 0
    /* RaceCar */ { 58, 1 },  // sheet row 7, col 2
    /* Truck   */ { 50, 2 },  // sheet row 6, cols 2-3
};

struct LaneConfig {
    int         row;
    int         direction; // +1 = right, -1 = left
    float       speed;     // pixels per second
    VehicleType type;
    int         count;
    float       spacing;   // pixels between vehicle starts
};

inline constexpr LaneConfig LANE_CONFIGS[] = {
    { 11, -1,  80.f, VehicleType::Car,    3, 250.f },
    { 10,  1, 150.f, VehicleType::RaceCar,4, 200.f },
    {  9, -1,  60.f, VehicleType::Truck,  2, 360.f },
    {  8,  1, 100.f, VehicleType::Car,    3, 250.f },
    {  7, -1, 130.f, VehicleType::RaceCar,3, 230.f },
};
inline constexpr int LANE_COUNT = 5;
