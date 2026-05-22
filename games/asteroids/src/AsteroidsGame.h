#pragma once
#include <engine/core/Application.h>
#include <engine/core/SaveData.h>
#include <engine/renderer/Renderer2D.h>
#include <engine/renderer/SpriteSheet.h>
#include <engine/renderer/Texture.h>
#include <engine/ui/Menu.h>
#include "Ship.h"
#include "Bullet.h"
#include "Asteroid.h"
#include <memory>
#include <optional>
#include <random>
#include <vector>

class AsteroidsGame : public Engine::Application {
public:
    AsteroidsGame();

    enum class Screen { Title, Playing, GameOver };

protected:
    void preStep(float dt)  override;
    void onUpdate(float dt) override;
    void onRender()         override;

private:
    static constexpr int   STARTING_ASTEROID_COUNT                = 4;
    static constexpr float STARTING_ASTEROID_MIN_DIST_FROM_PLAYER = 150.f;
    static constexpr int   STARTING_LIVES                         = 3;
    static constexpr int   MAX_ASTEROIDS_PER_WAVE                 = 12;
    static constexpr float WAVE_DELAY                             = 2.f;
    static constexpr float WAVE_SPAWN_MIN_DIST_FROM_SHIP          = 150.f;
    static constexpr int   SCORE_LARGE                            = 20;
    static constexpr int   SCORE_MEDIUM                           = 50;
    static constexpr int   SCORE_SMALL                            = 100;

    Engine::Renderer2D                        m_renderer;
    std::shared_ptr<Engine::SpriteSheet>      m_sheet;
    std::optional<Ship>                       m_ship;
    std::vector<std::unique_ptr<Bullet>>      m_bullets;
    std::vector<std::unique_ptr<Asteroid>>    m_asteroids;
    std::vector<std::unique_ptr<Asteroid>>    m_bgAsteroids;
    std::mt19937                              m_rng;
    Engine::SaveData                          m_saveData;
    Engine::Menu                             m_titleMenu;
    Screen                                    m_screen       = Screen::Title;
    int                                       m_lives        = STARTING_LIVES;
    int                                       m_score        = 0;
    int                                       m_highScore    = 0;
    int                                       m_wave         = 1;
    float                                     m_waveTimer    = -1.f;
    bool                                      m_newHighScore = false;

    // title screen helpers
    void spawnBgAsteroids();
    void updateTitleScreen(float dt);
    void renderTitleScreen();

    // onUpdate helpers
#ifdef ENABLE_DEV_KEYS
    void handleDevInput();
#endif
    void handleShipHit();
    void spawnAsteroidFragments();
    void removeDeadAsteroids();
    void advanceWaveIfCleared(float dt);
    void tryFireBullet();
    void removeDeadBullets();
    void restartGame();

    // onRender helpers
    void renderScore();
    void renderLivesHUD();
    void renderWaveAnnouncement();
    void renderGameOver();

    // Spawning
    void      spawnInitialAsteroidRing();
    void      spawnWave(int wave);
    glm::vec2 randomEdgePosition();

    int scoreForSize(AsteroidSize size) const;
};
