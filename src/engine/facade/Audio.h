/// @file Audio.h
/// @brief Engine audio facade — synthesized tones, noise, and .wav file playback.
///
/// All functions route through the engine's internal AudioManager.
/// Game code should not include AudioManager.h directly.
#pragma once

namespace Engine::Audio {

/// Play a synthesized sine-wave tone.
/// @param frequencyHz  Pitch in Hz (e.g. 440 = A4, 480 = B4).
/// @param durationSec  Duration in seconds.
/// @param amplitude    Volume in the range 0.0–1.0. Defaults to 0.4.
void playTone(float frequencyHz, float durationSec, float amplitude = 0.4f);

/// Play a white-noise burst.
/// @param durationSec  Duration in seconds.
/// @param amplitude    Volume in the range 0.0–1.0. Defaults to 0.4.
/// @param decayFactor  Per-sample amplitude multiplier for natural fade-out.
///                     Use `std::pow(0.001f, 1.f / (44100.f * fadeTimeSec))`
///                     to derive a value from a desired fade time.
///                     1.0 = no decay (flat burst).
void playNoise(float durationSec, float amplitude = 0.4f, float decayFactor = 1.f);

/// Start a sustained looping tone on a dedicated slot.
/// Slots are shared across all game objects; assign slot indices a
/// game-specific meaning (e.g. slot 0 = engine hum, slot 1 = shield).
/// @param slot         Voice slot index (0 to AudioManager::LOOPING_SLOT_COUNT-1).
/// @param frequencyHz  Pitch in Hz.
/// @param amplitude    Volume in the range 0.0–1.0.
void playLoopingTone(int slot, float frequencyHz, float amplitude);

/// Start a sustained looping noise on a dedicated slot.
/// @param slot       Voice slot index (0 to AudioManager::LOOPING_SLOT_COUNT-1).
/// @param amplitude  Volume in the range 0.0–1.0.
void playLoopingNoise(int slot, float amplitude);

/// Stop a looping voice started with playLoopingTone or playLoopingNoise.
/// Safe to call when the slot is already silent.
/// @param slot  Voice slot index to stop.
void stopLoopingVoice(int slot);

/// Opaque handle to a pre-loaded sound. Returned by loadSound; pass to playSound.
/// A default-constructed handle is invalid.
struct SoundHandle {
    int index = -1;
    bool valid() const;
};

/// Load a .wav file into memory for later playback.
/// @param path  Path to the .wav file, relative to the working directory.
/// @return      A handle for use with playSound. Check handle.valid() before use.
SoundHandle loadSound(const char* path);

/// Play a previously loaded sound, fire-and-forget.
/// @param handle  A handle returned by loadSound.
void playSound(SoundHandle handle);

/// Returns true if any voice is currently playing the given sound.
/// @param handle  A handle returned by loadSound.
bool isPlaying(SoundHandle handle);

} // namespace Engine::Audio
