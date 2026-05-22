#pragma once
#include "AsteroidsConfig.h"

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

// Indexed by UfoSize cast to int (Large=0, Small=1).
inline constexpr UfoConfig All[] = {
    //           renderSize      speed  aimVar    rate  frame score  cnt  spd   spVar  life  lifeVar  size
    /* Large */ { 32.f*SCALE,    80.f,  3.14159f, 2.0f,  44,  200,  18, 150.f, 70.f, 1.0f,  0.25f, 4.f*SCALE },
    /* Small */ { 16.f*SCALE,   120.f,  0.15f,    1.2f,  12, 1000,  10, 110.f, 70.f, 0.7f,  0.25f, 3.f*SCALE },
};

} // namespace UfoConfigs
