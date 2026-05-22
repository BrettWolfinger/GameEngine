#pragma once

namespace Engine {

class AudioManager {
public:
    static void init();
    static void shutdown();

    // Fire-and-forget sine tone. Picks a free voice from the pool.
    static void playTone(float frequencyHz, float durationSec, float amplitude = 0.4f);

    // Fire-and-forget white noise burst. decayFactor is a per-sample amplitude
    // multiplier — values slightly below 1.0 produce natural fade-outs.
    // Use std::pow(0.001f, 1.f / (44100.f * fadeTimeSec)) to compute decayFactor.
    static void playNoise(float durationSec, float amplitude = 0.4f, float decayFactor = 1.f);

    // Sustained looping voices on dedicated slots (0..LOOPING_SLOT_COUNT-1).
    // Game code assigns game-specific meaning to slot indices.
    static constexpr int LOOPING_SLOT_COUNT = 2;
    static void playLoopingTone (int slot, float frequencyHz, float amplitude);
    static void playLoopingNoise(int slot, float amplitude);
    static void stopLoopingVoice(int slot);
};

} // namespace Engine
