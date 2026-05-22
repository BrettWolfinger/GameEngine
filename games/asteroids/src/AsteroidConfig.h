#pragma once
#include "AsteroidsConfig.h"

struct AsteroidSizeConfig {
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
    //           count  speed  spVar  life  lifeVar  size         dur   amp   fade
    /* Large  */ { 20, 130.f, 70.f,  1.2f,  0.30f, 4.f*SCALE,  0.60f, 0.50f, 0.50f },
    /* Medium */ { 12, 100.f, 50.f,  0.8f,  0.20f, 3.f*SCALE,  0.35f, 0.40f, 0.25f },
    /* Small  */ {  6,  80.f, 40.f,  0.5f,  0.15f, 2.f*SCALE,  0.15f, 0.35f, 0.12f },
};

} // namespace AsteroidSizeConfigs
