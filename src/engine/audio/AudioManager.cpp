#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "AudioManager.h"
#include <atomic>
#include <cmath>
#include <algorithm>

namespace Engine {

static constexpr int SAMPLE_RATE = 44100;

static ma_device          s_device;
static bool               s_initialized = false;

// Written by game thread, read by audio thread — atomics for thread safety.
static std::atomic<float> s_freq      { 440.f };
static std::atomic<float> s_amplitude { 0.4f  };
static std::atomic<int>   s_framesLeft{ 0     };

// Phase is only ever touched by the audio thread.
static float s_phase = 0.f;

static void dataCallback(ma_device*, void* pOutput, const void*, ma_uint32 frameCount) {
    float* out       = static_cast<float*>(pOutput);
    int    remaining = s_framesLeft.load(std::memory_order_relaxed);
    float  freq      = s_freq.load(std::memory_order_relaxed);
    float  amp       = s_amplitude.load(std::memory_order_relaxed);

    for (ma_uint32 i = 0; i < frameCount; ++i) {
        if (remaining > 0) {
            out[i] = amp * std::sin(s_phase * 2.f * 3.14159265f);
            s_phase += freq / static_cast<float>(SAMPLE_RATE);
            if (s_phase >= 1.f) s_phase -= 1.f;
            --remaining;
        } else {
            out[i]  = 0.f;
            s_phase = 0.f; // reset to zero crossing so next tone starts cleanly
        }
    }

    s_framesLeft.store(remaining, std::memory_order_relaxed);
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
    s_freq.store(frequencyHz, std::memory_order_relaxed);
    s_amplitude.store(amplitude, std::memory_order_relaxed);
    s_framesLeft.store(static_cast<int>(durationSec * SAMPLE_RATE), std::memory_order_relaxed);
}

} // namespace Engine
