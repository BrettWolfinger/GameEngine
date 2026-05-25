#pragma once
#include "AsteroidsConfig.h"
#include <vector>

struct UfoConfig {
    float renderSize;
    float speed;
    float aimVariance;      // radians; ~pi = fully random, small value = tight cone toward player
    float fireRate;         // seconds between shots
    int   spriteFrame;
    int   score;
    // particles
    int   particleCount;
    float particleSpeed;
    float particleSpeedVariance;
    float particleLifetime;
    float particleLifetimeVariance;
    float particleSize;
};

namespace UfoConfigs {

// ---- shared tuning ----
inline constexpr float ZIGZAG_INTERVAL   = 1.5f;
inline constexpr float ZIGZAG_MIN_YSPEED = 20.f;
inline constexpr float ZIGZAG_MAX_YSPEED = 60.f;
inline constexpr float BEEP_INTERVAL     = 0.45f;
inline constexpr float BEEP_FREQUENCY_HI = 550.f;
inline constexpr float BEEP_FREQUENCY_LO = 400.f;
inline constexpr float BEEP_DURATION     = 0.25f;
inline constexpr float BEEP_AMPLITUDE    = 0.18f;
inline constexpr float NOISE_DURATION    = 0.40f;
inline constexpr float NOISE_AMPLITUDE   = 0.40f;
inline constexpr float NOISE_FADE_TIME   = 0.35f;  // seconds until explosion fades to near-silence
inline constexpr int   SPRITE_CELLS      = 2;

// Indexed by UfoSize cast to int (Large=0, Small=1). Populated by loadAllConfigs().
inline std::vector<UfoConfig> All;

} // namespace UfoConfigs
