#pragma once
#include <engine/Engine.h>
#include "PacmanEvents.h"

class PacmanAudio {
public:
    PacmanAudio();

private:
    Engine::Audio::SoundHandle m_chomp;
    Engine::Audio::SoundHandle m_eatGhost;
    Engine::Audio::SoundHandle m_death;
    Engine::Audio::SoundHandle m_beginning;
    Engine::Audio::SoundHandle m_extraPac;

    Engine::ListenerHandle m_dotHandle;
    Engine::ListenerHandle m_pelletHandle;
    Engine::ListenerHandle m_ghostHandle;
    Engine::ListenerHandle m_deathHandle;
    Engine::ListenerHandle m_levelClearedHandle;
    Engine::ListenerHandle m_startHandle;
    Engine::ListenerHandle m_extraLifeHandle;
};
