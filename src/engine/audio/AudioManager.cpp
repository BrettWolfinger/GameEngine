#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "AudioManager.h"
#include <atomic>
#include <cmath>
#include <algorithm>

namespace Engine {

static constexpr int SAMPLE_RATE        = 44100;
static constexpr int VOICE_COUNT        = 8;
// First LOOPING_SLOT_COUNT voices are reserved for sustained looping tones.
static constexpr int LOOPING_SLOTS      = AudioManager::LOOPING_SLOT_COUNT;
static constexpr int FREE_VOICE_START   = LOOPING_SLOTS;

enum class VoiceType : int { None = 0, Sine, Noise };

struct Voice {
    std::atomic<int>   type      { static_cast<int>(VoiceType::None) };
    std::atomic<float> freq      { 440.f };
    std::atomic<float> amplitude { 0.f   };
    std::atomic<float> decay     { 1.f   };
    std::atomic<int>   framesLeft{ 0     };
    float phase = 0.f;  // only touched by the audio thread
};

static ma_device s_device;
static bool      s_initialized = false;
static Voice     s_voices[VOICE_COUNT];

// Track per-slot active state so playLoopingTone is idempotent.
static bool s_loopingActive[LOOPING_SLOTS] = {};

// LCG noise generator — audio thread only.
static uint32_t s_noiseSeed = 12345;
static float nextNoiseSample() {
    s_noiseSeed = s_noiseSeed * 1664525u + 1013904223u;
    return static_cast<float>(static_cast<int32_t>(s_noiseSeed)) * (1.f / 2147483648.f);
}

static void dataCallback(ma_device*, void* pOutput, const void*, ma_uint32 frameCount) {
    float* out = static_cast<float*>(pOutput);
    for (ma_uint32 i = 0; i < frameCount; ++i) out[i] = 0.f;

    for (int v = 0; v < VOICE_COUNT; ++v) {
        Voice& voice = s_voices[v];
        int remaining = voice.framesLeft.load(std::memory_order_relaxed);
        if (remaining <= 0) continue;

        const VoiceType vtype = static_cast<VoiceType>(voice.type.load(std::memory_order_relaxed));
        float           amp   = voice.amplitude.load(std::memory_order_relaxed);
        const float     freq  = voice.freq.load(std::memory_order_relaxed);
        const float     decay = voice.decay.load(std::memory_order_relaxed);

        for (ma_uint32 i = 0; i < frameCount && remaining > 0; ++i) {
            float sample = 0.f;
            if (vtype == VoiceType::Sine) {
                sample = amp * std::sin(voice.phase * 6.28318530f);
                voice.phase += freq / static_cast<float>(SAMPLE_RATE);
                if (voice.phase >= 1.f) voice.phase -= 1.f;
            } else if (vtype == VoiceType::Noise) {
                sample = amp * nextNoiseSample();
            }
            out[i] += sample;
            amp *= decay;
            --remaining;
        }

        voice.amplitude.store(amp, std::memory_order_relaxed);
        voice.framesLeft.store(remaining, std::memory_order_relaxed);
        if (remaining <= 0) {
            voice.type.store(static_cast<int>(VoiceType::None), std::memory_order_relaxed);
            voice.phase = 0.f;
        }
    }

    // Soft clip to prevent distortion when voices overlap.
    for (ma_uint32 i = 0; i < frameCount; ++i)
        out[i] = std::clamp(out[i], -1.f, 1.f);
}

// Write ordering: auxiliary params first, framesLeft last. This ensures the
// audio thread sees a consistent state when a voice becomes active.
static void spawnVoice(int v, VoiceType type, float freq, float amp, float decay, float durationSec) {
    s_voices[v].freq.store(freq,                                               std::memory_order_relaxed);
    s_voices[v].amplitude.store(amp,                                           std::memory_order_relaxed);
    s_voices[v].decay.store(decay,                                             std::memory_order_relaxed);
    s_voices[v].type.store(static_cast<int>(type),                            std::memory_order_relaxed);
    s_voices[v].framesLeft.store(static_cast<int>(durationSec * SAMPLE_RATE), std::memory_order_relaxed);
}

static int findFreeVoice() {
    for (int i = FREE_VOICE_START; i < VOICE_COUNT; ++i) {
        if (s_voices[i].framesLeft.load(std::memory_order_relaxed) <= 0)
            return i;
    }
    // All busy — steal the voice with the fewest remaining frames.
    int best  = FREE_VOICE_START;
    int least = s_voices[best].framesLeft.load(std::memory_order_relaxed);
    for (int i = FREE_VOICE_START + 1; i < VOICE_COUNT; ++i) {
        int f = s_voices[i].framesLeft.load(std::memory_order_relaxed);
        if (f < least) { least = f; best = i; }
    }
    return best;
}

void AudioManager::init() {
    ma_device_config cfg  = ma_device_config_init(ma_device_type_playback);
    cfg.playback.format   = ma_format_f32;
    cfg.playback.channels = 1;
    cfg.sampleRate        = SAMPLE_RATE;
    cfg.dataCallback      = dataCallback;

    if (ma_device_init(nullptr, &cfg, &s_device) != MA_SUCCESS)
        return; // audio is non-critical — fail silently

    ma_device_start(&s_device);
    s_initialized = true;
}

void AudioManager::shutdown() {
    if (s_initialized) {
        ma_device_uninit(&s_device);
        s_initialized = false;
    }
}

void AudioManager::playTone(float frequencyHz, float durationSec, float amplitude) {
    if (!s_initialized) return;
    spawnVoice(findFreeVoice(), VoiceType::Sine, frequencyHz, amplitude, 1.f, durationSec);
}

void AudioManager::playNoise(float durationSec, float amplitude, float decayFactor) {
    if (!s_initialized) return;
    spawnVoice(findFreeVoice(), VoiceType::Noise, 0.f, amplitude, decayFactor, durationSec);
}

void AudioManager::playLoopingTone(int slot, float frequencyHz, float amplitude) {
    if (!s_initialized || slot < 0 || slot >= LOOPING_SLOTS) return;
    if (s_loopingActive[slot]) return;
    s_loopingActive[slot] = true;
    spawnVoice(slot, VoiceType::Sine, frequencyHz, amplitude, 1.f, 48000.f); // 13+ hours
}

void AudioManager::playLoopingNoise(int slot, float amplitude) {
    if (!s_initialized || slot < 0 || slot >= LOOPING_SLOTS) return;
    if (s_loopingActive[slot]) return;
    s_loopingActive[slot] = true;
    spawnVoice(slot, VoiceType::Noise, 0.f, amplitude, 1.f, 48000.f); // 13+ hours
}

void AudioManager::stopLoopingVoice(int slot) {
    if (!s_initialized || slot < 0 || slot >= LOOPING_SLOTS) return;
    if (!s_loopingActive[slot]) return;
    s_loopingActive[slot] = false;
    s_voices[slot].type.store(static_cast<int>(VoiceType::None), std::memory_order_relaxed);
    s_voices[slot].framesLeft.store(0, std::memory_order_relaxed);
}

} // namespace Engine
