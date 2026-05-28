#include "PacmanAudio.h"

PacmanAudio::PacmanAudio() {
    m_chomp     = Engine::Audio::loadSound("games/pacman/assets/audio/pacman_chomp.wav");
    m_eatGhost  = Engine::Audio::loadSound("games/pacman/assets/audio/pacman_eatghost.wav");
    m_death     = Engine::Audio::loadSound("games/pacman/assets/audio/pacman_death.wav");
    m_beginning = Engine::Audio::loadSound("games/pacman/assets/audio/pacman_beginning.wav");
    m_extraPac  = Engine::Audio::loadSound("games/pacman/assets/audio/pacman_extrapac.wav");

    m_dotHandle = Engine::Events::on<DotEaten>([this](const DotEaten&) {
        if (!Engine::Audio::isPlaying(m_chomp))
            Engine::Audio::playSound(m_chomp);
    });

    m_pelletHandle = Engine::Events::on<PowerPelletEaten>([this](const PowerPelletEaten&) {
        if (!Engine::Audio::isPlaying(m_chomp))
            Engine::Audio::playSound(m_chomp);
    });

    m_ghostHandle = Engine::Events::on<GhostEaten>([this](const GhostEaten&) {
        Engine::Audio::playSound(m_eatGhost);
    });

    m_deathHandle = Engine::Events::on<DeathStarted>([this](const DeathStarted&) {
        Engine::Audio::playSound(m_death);
    });

    m_levelClearedHandle = Engine::Events::on<LevelCleared>([this](const LevelCleared&) {
        Engine::Audio::playSound(m_beginning);
    });

    m_startHandle = Engine::Events::on<GameStarted>([this](const GameStarted&) {
        Engine::Audio::playSound(m_beginning);
    });

    m_extraLifeHandle = Engine::Events::on<ExtraLifeAwarded>([this](const ExtraLifeAwarded&) {
        Engine::Audio::playSound(m_extraPac);
    });
}
