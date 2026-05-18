#pragma once

namespace Engine {

class AudioManager {
public:
    static void init();
    static void shutdown();

    // Plays a sine-wave tone on the audio thread. Fire-and-forget; a new call
    // interrupts any tone currently playing.
    static void playTone(float frequencyHz, float durationSec, float amplitude = 0.4f);
};

} // namespace Engine
