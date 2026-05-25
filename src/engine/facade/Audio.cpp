#include "Audio.h"
#include <engine/core/Services.h>
#include <engine/audio/AudioManager.h>

namespace Engine::Audio {

void playTone(float frequencyHz, float durationSec, float amplitude) {
    Services::audio().playTone(frequencyHz, durationSec, amplitude);
}

void playNoise(float durationSec, float amplitude, float decayFactor) {
    Services::audio().playNoise(durationSec, amplitude, decayFactor);
}

void playLoopingTone(int slot, float frequencyHz, float amplitude) {
    Services::audio().playLoopingTone(slot, frequencyHz, amplitude);
}

void playLoopingNoise(int slot, float amplitude) {
    Services::audio().playLoopingNoise(slot, amplitude);
}

void stopLoopingVoice(int slot) {
    Services::audio().stopLoopingVoice(slot);
}

} // namespace Engine::Audio
