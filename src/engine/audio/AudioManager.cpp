#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "AudioManager.h"
#include <cmath>
#include <algorithm>
#include <cstring>

namespace Engine {

AudioManager::AudioManager()  = default;
AudioManager::~AudioManager() = default;

// ---- audio thread -----------------------------------------------------------

float AudioManager::nextNoiseSample() {
    m_noiseSeed = m_noiseSeed * 1664525u + 1013904223u;
    return static_cast<float>(static_cast<int32_t>(m_noiseSeed)) * (1.f / 2147483648.f);
}

void AudioManager::dataCallback(void* pOutput, uint32_t frameCount) {
    float* out = static_cast<float*>(pOutput);
    for (uint32_t i = 0; i < frameCount; ++i) out[i] = 0.f;

    for (int v = 0; v < VOICE_COUNT; ++v) {
        Voice& voice = m_voices[v];
        int remaining = voice.framesLeft.load(std::memory_order_relaxed);
        if (remaining <= 0) continue;

        // Use acquire on type so we see all PCM fields written before activation.
        const VoiceType vtype = static_cast<VoiceType>(voice.type.load(std::memory_order_acquire));
        float           amp   = voice.amplitude.load(std::memory_order_relaxed);
        const float     freq  = voice.freq.load(std::memory_order_relaxed);
        const float     decay = voice.decay.load(std::memory_order_relaxed);

        if (vtype == VoiceType::PCM) {
            const float* pcm      = voice.pcmData.load(std::memory_order_relaxed);
            const int    channels = voice.pcmChannels;
            int          readPos  = voice.pcmReadPos;

            for (uint32_t i = 0; i < frameCount && remaining > 0; ++i) {
                float sample = 0.f;
                if (channels == 1) {
                    sample = pcm[readPos];
                } else {
                    // Mix all channels down to mono for the single-channel device output.
                    for (int ch = 0; ch < channels; ++ch)
                        sample += pcm[readPos * channels + ch];
                    sample /= static_cast<float>(channels);
                }
                out[i] += sample * amp;
                ++readPos;
                --remaining;
            }

            voice.pcmReadPos = readPos;
            voice.framesLeft.store(remaining, std::memory_order_relaxed);
            if (remaining <= 0) {
                voice.type.store(static_cast<int>(VoiceType::None), std::memory_order_relaxed);
                voice.pcmReadPos = 0;
            }
        } else {
            for (uint32_t i = 0; i < frameCount && remaining > 0; ++i) {
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
    }

    // Soft clip to prevent distortion when voices overlap.
    for (uint32_t i = 0; i < frameCount; ++i)
        out[i] = std::clamp(out[i], -1.f, 1.f);
}

void AudioManager::dataCallbackThunk(ma_device* device, void* pOutput, const void*, uint32_t frameCount) {
    static_cast<AudioManager*>(device->pUserData)->dataCallback(pOutput, frameCount);
}

// ---- game thread ------------------------------------------------------------

// Write ordering: auxiliary params first, framesLeft last. This ensures the
// audio thread sees a consistent state when a voice becomes active.
void AudioManager::spawnVoice(int v, VoiceType type, float freq, float amp, float decay, float durationSec) {
    m_voices[v].freq.store(freq,                                               std::memory_order_relaxed);
    m_voices[v].amplitude.store(amp,                                           std::memory_order_relaxed);
    m_voices[v].decay.store(decay,                                             std::memory_order_relaxed);
    m_voices[v].type.store(static_cast<int>(type),                            std::memory_order_relaxed);
    m_voices[v].framesLeft.store(static_cast<int>(durationSec * SAMPLE_RATE), std::memory_order_relaxed);
}

int AudioManager::findFreeVoice() {
    for (int i = FREE_VOICE_START; i < VOICE_COUNT; ++i) {
        if (m_voices[i].framesLeft.load(std::memory_order_relaxed) <= 0)
            return i;
    }
    // All busy — steal the voice with the fewest remaining frames.
    int best  = FREE_VOICE_START;
    int least = m_voices[best].framesLeft.load(std::memory_order_relaxed);
    for (int i = FREE_VOICE_START + 1; i < VOICE_COUNT; ++i) {
        int f = m_voices[i].framesLeft.load(std::memory_order_relaxed);
        if (f < least) { least = f; best = i; }
    }
    return best;
}

// ---- lifecycle --------------------------------------------------------------

void AudioManager::init() {
    m_device = new ma_device;

    ma_device_config cfg  = ma_device_config_init(ma_device_type_playback);
    cfg.playback.format   = ma_format_f32;
    cfg.playback.channels = 1;
    cfg.sampleRate        = SAMPLE_RATE;
    cfg.dataCallback      = dataCallbackThunk;
    cfg.pUserData         = this;

    if (ma_device_init(nullptr, &cfg, m_device) != MA_SUCCESS) {
        delete m_device;
        m_device = nullptr;
        return; // audio is non-critical — fail silently
    }

    ma_device_start(m_device);
    m_initialized = true;
}

void AudioManager::shutdown() {
    if (m_initialized) {
        ma_device_uninit(m_device);
        m_initialized = false;
    }
    delete m_device;
    m_device = nullptr;
}

// ---- public API -------------------------------------------------------------

void AudioManager::playTone(float frequencyHz, float durationSec, float amplitude) {
    if (!m_initialized) return;
    spawnVoice(findFreeVoice(), VoiceType::Sine, frequencyHz, amplitude, 1.f, durationSec);
}

void AudioManager::playNoise(float durationSec, float amplitude, float decayFactor) {
    if (!m_initialized) return;
    spawnVoice(findFreeVoice(), VoiceType::Noise, 0.f, amplitude, decayFactor, durationSec);
}

void AudioManager::playLoopingTone(int slot, float frequencyHz, float amplitude) {
    if (!m_initialized || slot < 0 || slot >= LOOPING_SLOT_COUNT) return;
    if (m_loopingActive[slot]) return;
    m_loopingActive[slot] = true;
    spawnVoice(slot, VoiceType::Sine, frequencyHz, amplitude, 1.f, 48000.f);
}

void AudioManager::playLoopingNoise(int slot, float amplitude) {
    if (!m_initialized || slot < 0 || slot >= LOOPING_SLOT_COUNT) return;
    if (m_loopingActive[slot]) return;
    m_loopingActive[slot] = true;
    spawnVoice(slot, VoiceType::Noise, 0.f, amplitude, 1.f, 48000.f);
}

void AudioManager::stopLoopingVoice(int slot) {
    if (!m_initialized || slot < 0 || slot >= LOOPING_SLOT_COUNT) return;
    if (!m_loopingActive[slot]) return;
    m_loopingActive[slot] = false;
    m_voices[slot].type.store(static_cast<int>(VoiceType::None), std::memory_order_relaxed);
    m_voices[slot].framesLeft.store(0, std::memory_order_relaxed);
}

int AudioManager::loadSound(const char* path) {
    ma_decoder_config cfg = ma_decoder_config_init(ma_format_f32, 0, SAMPLE_RATE);
    ma_uint64 frameCount  = 0;
    void*     pData       = nullptr;
    if (ma_decode_file(path, &cfg, &frameCount, &pData) != MA_SUCCESS)
        return -1;

    SoundBuffer buf;
    buf.frameCount   = static_cast<int>(frameCount);
    buf.channelCount = static_cast<int>(cfg.channels);
    buf.samples.resize(frameCount * cfg.channels);
    std::memcpy(buf.samples.data(), pData, frameCount * cfg.channels * sizeof(float));
    ma_free(pData, nullptr);

    m_soundBuffers.push_back(std::move(buf));
    return static_cast<int>(m_soundBuffers.size()) - 1;
}

void AudioManager::playSound(int soundIndex) {
    if (!m_initialized) return;
    if (soundIndex < 0 || soundIndex >= static_cast<int>(m_soundBuffers.size())) return;
    int v = findFreeVoice();
    if (v < 0) return;
    spawnPCMVoice(v, m_soundBuffers[soundIndex]);
}

bool AudioManager::isPlaying(int soundIndex) const {
    if (soundIndex < 0 || soundIndex >= static_cast<int>(m_soundBuffers.size())) return false;
    const float* data = m_soundBuffers[soundIndex].samples.data();
    for (int v = FREE_VOICE_START; v < VOICE_COUNT; ++v) {
        if (m_voices[v].framesLeft.load(std::memory_order_relaxed) > 0 &&
            static_cast<VoiceType>(m_voices[v].type.load(std::memory_order_relaxed)) == VoiceType::PCM &&
            m_voices[v].pcmData.load(std::memory_order_relaxed) == data)
            return true;
    }
    return false;
}

void AudioManager::spawnPCMVoice(int v, const SoundBuffer& buf) {
    // Write all PCM fields before activating via type (release store ensures ordering).
    m_voices[v].pcmData.store(buf.samples.data(), std::memory_order_relaxed);
    m_voices[v].pcmFrameCount = buf.frameCount;
    m_voices[v].pcmChannels   = buf.channelCount;
    m_voices[v].pcmReadPos    = 0;
    m_voices[v].amplitude.store(1.f,             std::memory_order_relaxed);
    m_voices[v].framesLeft.store(buf.frameCount, std::memory_order_relaxed);
    m_voices[v].type.store(static_cast<int>(VoiceType::PCM), std::memory_order_release);
}

} // namespace Engine
