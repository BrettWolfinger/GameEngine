#pragma once
#include <cstdint>

inline constexpr int      W             = 800;
inline constexpr int      H             = 800;
inline constexpr float    SCALE         = 1.f;

inline constexpr uint32_t kBulletLayer   = 1 << 0;
inline constexpr uint32_t kAsteroidLayer = 1 << 1;
inline constexpr uint32_t kShipLayer     = 1 << 2;
