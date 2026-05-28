# Audio System

## Overview

The audio system provides two modes of playback: synthesized waveforms (sine tones and noise bursts) generated in real time, and pre-loaded PCM buffers decoded from .wav files. Both share the same fixed voice pool and mix together in the audio callback.

All audio is accessible through the `Engine::Audio` facade. Do not include `AudioManager.h` directly from game code.

---

## Voice Pool

The engine maintains a pool of 8 voices. Two slots (0–1) are reserved for sustained looping voices. The remaining six are fire-and-forget voices shared across all playback types.

When all fire-and-forget voices are busy, the engine steals the one with the fewest frames remaining rather than silently dropping the new sound.

---

## Synthesized Audio

Synthesized voices are useful for simple beep-style effects and do not require any file loading.

```cpp
// One-shot sine tone: frequency (Hz), duration (sec), amplitude (0.0–1.0)
Engine::Audio::playTone(440.f, 0.15f);
Engine::Audio::playTone(880.f, 0.05f, 0.6f);

// White noise burst with optional amplitude decay
Engine::Audio::playNoise(0.3f, 0.4f);
Engine::Audio::playNoise(0.4f, 0.4f, std::pow(0.001f, 1.f / (44100.f * 0.3f)));

// Sustained looping tone on a dedicated slot (0 or 1)
Engine::Audio::playLoopingTone(0, 220.f, 0.3f);
Engine::Audio::stopLoopingVoice(0);

// Sustained looping noise
Engine::Audio::playLoopingNoise(1, 0.2f);
Engine::Audio::stopLoopingVoice(1);
```

Looping slots are shared across all game objects. Assign slot indices a game-specific meaning (e.g. slot 0 = engine hum, slot 1 = shield drone) and document them in `GameConstants.h`.

---

## File-Based Audio

.wav files are decoded upfront at load time. Each loaded file produces a `SoundHandle` — a lightweight token used for playback and queries.

### Loading

Load sounds during initialization, not during gameplay:

```cpp
class PacmanAudio {
    Engine::Audio::SoundHandle m_chomp;
    Engine::Audio::SoundHandle m_death;
};

PacmanAudio::PacmanAudio() {
    m_chomp = Engine::Audio::loadSound("games/pacman/assets/audio/pacman_chomp.wav");
    m_death = Engine::Audio::loadSound("games/pacman/assets/audio/pacman_death.wav");
}
```

`loadSound` returns an invalid handle (`handle.valid() == false`) if the file is missing or cannot be decoded. Audio calls with invalid handles are safe no-ops.

Sounds are decoded to float32 PCM at the device sample rate (44100 Hz) via miniaudio's built-in resampler. All standard .wav formats and sample rates are supported.

### Playback

```cpp
Engine::Audio::playSound(m_chomp);
```

Fire-and-forget. Picks a free voice from the pool and starts playback. Returns immediately; the audio thread drives the buffer from that point on.

### Querying

```cpp
if (!Engine::Audio::isPlaying(m_chomp))
    Engine::Audio::playSound(m_chomp);
```

`isPlaying` returns true if any voice is currently playing that buffer. Use it to prevent the same sound from stacking — useful for rapid events like dot eating where the previous chomp may not have finished.

---

## Wiring Audio to Game Events

The intended pattern is a dedicated audio class that subscribes to game events, keeping audio logic entirely separate from game logic:

```cpp
PacmanAudio::PacmanAudio() {
    m_chomp    = Engine::Audio::loadSound("games/pacman/assets/audio/pacman_chomp.wav");
    m_eatGhost = Engine::Audio::loadSound("games/pacman/assets/audio/pacman_eatghost.wav");

    m_dotHandle = Engine::Events::on<DotEaten>([this](const DotEaten&) {
        if (!Engine::Audio::isPlaying(m_chomp))
            Engine::Audio::playSound(m_chomp);
    });

    m_ghostHandle = Engine::Events::on<GhostEaten>([this](const GhostEaten&) {
        Engine::Audio::playSound(m_eatGhost);
    });
}
```

The game emits events; the audio class reacts. Neither references the other. See [Event System](event-system.md) for the full subscription model.
