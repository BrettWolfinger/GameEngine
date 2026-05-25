#pragma once
#include <engine/core/Services.h>
#include <engine/audio/AudioManager.h>

namespace Engine::Audio {

inline void playTone(float frequencyHz, float durationSec, float amplitude = 0.4f) {
    Services::audio().playTone(frequencyHz, durationSec, amplitude);
}

inline void playNoise(float durationSec, float amplitude = 0.4f, float decayFactor = 1.f) {
    Services::audio().playNoise(durationSec, amplitude, decayFactor);
}

inline void playLoopingTone(int slot, float frequencyHz, float amplitude) {
    Services::audio().playLoopingTone(slot, frequencyHz, amplitude);
}

inline void playLoopingNoise(int slot, float amplitude) {
    Services::audio().playLoopingNoise(slot, amplitude);
}

inline void stopLoopingVoice(int slot) {
    Services::audio().stopLoopingVoice(slot);
}

} // namespace Engine::Audio
