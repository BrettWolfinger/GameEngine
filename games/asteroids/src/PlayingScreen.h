#pragma once
#include "GameContext.h"

class PlayingScreen {
public:
    explicit PlayingScreen(GameContext& ctx);

    void   onEnter();
    void   preStep(float dt);
    Screen update(float dt);
    void   render();

private:
    static constexpr int   STARTING_ASTEROID_COUNT                = 4;
    static constexpr float STARTING_ASTEROID_MIN_DIST_FROM_PLAYER = 150.f;
    static constexpr int   MAX_ASTEROIDS_PER_WAVE                 = 12;
    static constexpr float WAVE_DELAY                             = 2.f;
    static constexpr float WAVE_SPAWN_MIN_DIST_FROM_SHIP          = 150.f;
    static constexpr float UFO_INITIAL_SPAWN_DELAY                = 15.f;
    static constexpr float UFO_RESPAWN_DELAY                      = 20.f;

    GameContext& m_ctx;

#ifdef ENABLE_DEV_KEYS
    void handleDevInput();
#endif
    void handleShipHit();
    void spawnAsteroidFragments();
    void removeDeadAsteroids();
    void advanceWaveIfCleared(float dt);
    void tryFireBullet();
    void removeDeadBullets();
    void handleUfoState(float dt);
    void clearUfo(float respawnDelay);
    void spawnUfo();
    void trySpawnUfoBullet();
    void removeDeadUfoBullets();

    void renderScore();
    void renderLivesHUD();
    void renderWaveAnnouncement();

    void      awardScore(int pts);
    void      spawnInitialAsteroidRing();
    void      spawnWave(int wave);
    glm::vec2 randomEdgePosition();
};
