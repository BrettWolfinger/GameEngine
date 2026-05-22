#include "AsteroidsGame.h"
#include "AsteroidsConfig.h"
#include "ShipConfig.h"
#include <engine/renderer/Texture.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <algorithm>
#include <cmath>

static GameContext makeContext(
    Engine::Renderer2D& renderer,
    std::shared_ptr<Engine::SpriteSheet>& sheet,
    std::mt19937& rng,
    Engine::SaveData& saveData,
    Engine::Menu& titleMenu,
    std::vector<std::unique_ptr<Asteroid>>& bgAsteroids,
    std::optional<Ship>& ship,
    std::vector<std::unique_ptr<Bullet>>& bullets,
    std::vector<std::unique_ptr<Asteroid>>& asteroids,
    int& score, int& highScore, int& lives, int& wave,
    float& waveTimer, bool& newHighScore, int& selectedShip)
{
    return GameContext{
        renderer, sheet, rng, saveData, titleMenu,
        bgAsteroids, ship, bullets, asteroids,
        score, highScore, lives, wave, waveTimer, newHighScore, selectedShip
    };
}

AsteroidsGame::AsteroidsGame()
    : Engine::Application("Asteroids", W, H)
    , m_rng(std::random_device{}())
    , m_titleMenu({"PLAY", "EXIT"}, 3.f)
    , m_ctx(makeContext(m_renderer, m_sheet, m_rng, m_saveData, m_titleMenu,
                        m_bgAsteroids, m_ship, m_bullets, m_asteroids,
                        m_score, m_highScore, m_lives, m_wave,
                        m_waveTimer, m_newHighScore, m_selectedShip))
    , m_titleScreen(m_ctx)
    , m_shipSelectScreen(m_ctx)
    , m_playingScreen(m_ctx)
    , m_gameOverScreen(m_ctx)
{
    auto texture = std::make_shared<Engine::Texture>("games/asteroids/assets/asteroids-arcade.png");
    m_sheet      = std::make_shared<Engine::SpriteSheet>(texture, 16, 16);

    m_saveData  = Engine::SaveData::load("asteroids");
    m_highScore = m_saveData.getInt("high_score", 0);

    spawnBgAsteroids();
}

// ---- core loop --------------------------------------------------------------

void AsteroidsGame::preStep(float dt) {
    switch (m_screen) {
        case Screen::Title:      m_titleScreen.preStep(dt);      break;
        case Screen::ShipSelect: m_shipSelectScreen.preStep(dt); break;
        case Screen::Playing:    m_playingScreen.preStep(dt);    break;
        default: break;
    }
}

void AsteroidsGame::onUpdate(float dt) {
    Screen next = m_screen;
    switch (m_screen) {
        case Screen::Title:      next = m_titleScreen.update(dt);      break;
        case Screen::ShipSelect: next = m_shipSelectScreen.update(dt); break;
        case Screen::Playing:    next = m_playingScreen.update(dt);    break;
        case Screen::GameOver:   next = m_gameOverScreen.update(dt);   break;
        default: break;
    }
    if (next != m_screen) transitionTo(next);
}

void AsteroidsGame::onRender() {
    m_renderer.beginScene(W, H);
    switch (m_screen) {
        case Screen::Title:      m_titleScreen.render();      break;
        case Screen::ShipSelect: m_shipSelectScreen.render(); break;
        case Screen::Playing:    m_playingScreen.render();    break;
        case Screen::GameOver:   m_playingScreen.render();    // game world stays visible
                                 m_gameOverScreen.render();   break;
        default: break;
    }
}

// ---- transitions ------------------------------------------------------------

void AsteroidsGame::transitionTo(Screen next) {
    if (next == Screen::Quit) { quit(); return; }

    if (next == Screen::Playing && m_screen == Screen::ShipSelect)
        startGame();

    if (next == Screen::ShipSelect && m_screen == Screen::GameOver)
        resetForRestart();

    m_screen = next;
}

void AsteroidsGame::startGame() {
    m_ship.emplace(m_sheet, ShipConfigs::All[m_selectedShip]);
    spawnInitialAsteroidRing();
}

void AsteroidsGame::resetForRestart() {
    m_asteroids.clear();
    m_bullets.clear();
    m_ship.reset();
    m_score        = 0;
    m_lives        = 3;
    m_wave         = 1;
    m_waveTimer    = -1.f;
    m_newHighScore = false;
}

// ---- spawning ---------------------------------------------------------------

void AsteroidsGame::spawnBgAsteroids() {
    static constexpr int COUNT = 8;
    for (int i = 0; i < COUNT; ++i) {
        glm::vec2 pos = {
            std::uniform_real_distribution<float>(0.f, static_cast<float>(W))(m_rng),
            std::uniform_real_distribution<float>(0.f, static_cast<float>(H))(m_rng)
        };
        m_bgAsteroids.push_back(Asteroid::spawnLarge(pos, m_rng));
    }
}

void AsteroidsGame::spawnInitialAsteroidRing() {
    const glm::vec2 playerStart(W * 0.5f, H * 0.5f);

    for (int i = 0; i < STARTING_ASTEROID_COUNT; ++i) {
        const float baseAngle   = (glm::two_pi<float>() / STARTING_ASTEROID_COUNT) * i;
        const float jitter      = std::uniform_real_distribution<float>(
                                      -glm::pi<float>() / 6.f,
                                       glm::pi<float>() / 6.f)(m_rng);
        const float spawnRadius = STARTING_ASTEROID_MIN_DIST_FROM_PLAYER
                                + std::uniform_real_distribution<float>(0.f, 120.f)(m_rng);

        glm::vec2 pos = playerStart + glm::vec2(std::cos(baseAngle + jitter),
                                                std::sin(baseAngle + jitter)) * spawnRadius;
        pos.x = std::clamp(pos.x, 32.f, static_cast<float>(W) - 32.f);
        pos.y = std::clamp(pos.y, 32.f, static_cast<float>(H) - 32.f);

        m_asteroids.push_back(Asteroid::spawnLarge(pos, m_rng));
    }
}
