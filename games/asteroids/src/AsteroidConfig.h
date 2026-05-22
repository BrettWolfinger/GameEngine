#pragma once
#include "AsteroidsConfig.h"

struct AsteroidSizeConfig {
    // rendering
    int cellCount;
    // Frame table indexed as [variant][fragment_idx].
    // Large:  only [0][0] is used (one variant, no fragments).
    // Medium: [variant][0] — one frame per variant, no per-fragment variation.
    // Small:  [group][idx] — full 4×4 table (group inherited from parent medium).
    int frames[4][4];
    // score
    int score;
    // particles
    int   particleCount;
    float particleSpeed;
    float particleSpeedVariance;
    float particleLifetime;
    float particleLifetimeVariance;
    float particleSize;
    // audio
    float noiseDuration;
    float noiseAmplitude;
    float noiseFadeTime;   // seconds until blast fades to near-silence
};

// Indexed by AsteroidSize cast to int (Large=0, Medium=1, Small=2).
namespace AsteroidSizeConfigs {

inline constexpr AsteroidSizeConfig All[] = {
    /* Large  */ {
        /* cellCount */ 4,
        /* frames    */ { {196,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0} },
        /* score     */ 20,
        /* particles */ 20, 130.f, 70.f, 1.2f, 0.30f, 4.f*SCALE,
        /* audio     */ 0.60f, 0.50f, 0.50f,
    },
    /* Medium */ {
        /* cellCount */ 2,
        /* frames    */ { {200,0,0,0}, {202,0,0,0}, {232,0,0,0}, {234,0,0,0} },
        /* score     */ 50,
        /* particles */ 12, 100.f, 50.f, 0.8f, 0.20f, 3.f*SCALE,
        /* audio     */ 0.35f, 0.40f, 0.25f,
    },
    /* Small  */ {
        /* cellCount */ 1,
        /* frames    */ { {204,205,220,221}, {206,207,222,223}, {236,237,252,253}, {238,239,254,255} },
        /* score     */ 100,
        /* particles */ 6, 80.f, 40.f, 0.5f, 0.15f, 2.f*SCALE,
        /* audio     */ 0.15f, 0.35f, 0.12f,
    },
};

} // namespace AsteroidSizeConfigs
