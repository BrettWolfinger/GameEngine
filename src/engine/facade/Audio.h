#pragma once

namespace Engine::Audio {

void playTone       (float frequencyHz, float durationSec, float amplitude = 0.4f);
void playNoise      (float durationSec, float amplitude = 0.4f, float decayFactor = 1.f);
void playLoopingTone(int slot, float frequencyHz, float amplitude);
void playLoopingNoise(int slot, float amplitude);
void stopLoopingVoice(int slot);

} // namespace Engine::Audio
