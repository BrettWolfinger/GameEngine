#pragma once
#include "GameConstants.h"
#include <vector>

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

// Indexed by AsteroidSize cast to int (Large=0, Medium=1, Small=2). Populated by loadAllConfigs().
namespace AsteroidSizeConfigs {

inline std::vector<AsteroidSizeConfig> All;

} // namespace AsteroidSizeConfigs
