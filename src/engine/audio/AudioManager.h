#pragma once
#ifndef ENGINE_INTERNAL
#  error "engine/audio/AudioManager.h is an engine-internal header. Include <engine/Engine.h> instead."
#endif
#include <atomic>
#include <deque>
#include <vector>

// Forward-declare miniaudio device type to avoid pulling the heavy header here.
struct ma_device;

namespace Engine {

class AudioManager {
public:
    AudioManager();
    ~AudioManager();
    AudioManager(const AudioManager&)            = delete;
    AudioManager& operator=(const AudioManager&) = delete;

    void init();
    void shutdown();

    // Fire-and-forget sine tone. Picks a free voice from the pool.
    void playTone(float frequencyHz, float durationSec, float amplitude = 0.4f);

    // Fire-and-forget white noise burst. decayFactor is a per-sample amplitude
    // multiplier — values slightly below 1.0 produce natural fade-outs.
    // Use std::pow(0.001f, 1.f / (44100.f * fadeTimeSec)) to compute decayFactor.
    void playNoise(float durationSec, float amplitude = 0.4f, float decayFactor = 1.f);

    // Sustained looping voices on dedicated slots (0..LOOPING_SLOT_COUNT-1).
    // Game code assigns game-specific meaning to slot indices.
    static constexpr int LOOPING_SLOT_COUNT = 2;
    void playLoopingTone (int slot, float frequencyHz, float amplitude);
    void playLoopingNoise(int slot, float amplitude);
    void stopLoopingVoice(int slot);

    // Load a .wav file into memory. Returns index into internal buffer store; -1 on failure.
    int  loadSound(const char* path);
    // Play a previously loaded sound. soundIndex must be a value returned by loadSound.
    void playSound(int soundIndex);
    // Returns true if any voice is currently playing the given sound.
    bool isPlaying(int soundIndex) const;

private:
    static constexpr int SAMPLE_RATE      = 44100;
    static constexpr int VOICE_COUNT      = 8;
    static constexpr int FREE_VOICE_START = LOOPING_SLOT_COUNT;

    enum class VoiceType : int { None = 0, Sine, Noise, PCM };

    struct SoundBuffer {
        std::vector<float> samples;    // interleaved float32 PCM, resampled to SAMPLE_RATE
        int frameCount   = 0;
        int channelCount = 0;
    };

    struct Voice {
        std::atomic<int>          type       { static_cast<int>(VoiceType::None) };
        std::atomic<float>        freq       { 440.f  };
        std::atomic<float>        amplitude  { 0.f    };
        std::atomic<float>        decay      { 1.f    };
        std::atomic<int>          framesLeft { 0      };
        float                     phase      = 0.f;   // audio thread only

        // PCM fields — set by game thread before activation, read by audio thread after.
        std::atomic<const float*> pcmData      { nullptr };
        int                       pcmFrameCount{ 0 };    // immutable after load
        int                       pcmChannels  { 1 };    // immutable after load
        int                       pcmReadPos   { 0 };    // audio thread only
    };

    ma_device*               m_device       = nullptr;
    bool                     m_initialized  = false;
    bool                     m_loopingActive[LOOPING_SLOT_COUNT] = {};
    Voice                    m_voices[VOICE_COUNT];
    uint32_t                 m_noiseSeed = 12345; // audio thread only
    std::deque<SoundBuffer> m_soundBuffers;

    void  dataCallback(void* pOutput, uint32_t frameCount);
    float nextNoiseSample();
    void  spawnVoice(int v, VoiceType type, float freq, float amp, float decay, float durationSec);
    void  spawnPCMVoice(int v, const SoundBuffer& buf);
    int   findFreeVoice();

    static void dataCallbackThunk(ma_device*, void* pOutput, const void*, uint32_t frameCount);
};

} // namespace Engine
