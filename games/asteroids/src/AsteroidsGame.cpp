#include "AsteroidsGame.h"
#include "AsteroidsConfig.h"
#include "AsteroidsConfigs.h"
#include <engine/Engine.h>
#include <engine/renderer/Texture.h>

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
    std::optional<UFO>& ufo,
    std::vector<std::unique_ptr<Bullet>>& ufoBullets,
    float& ufoSpawnTimer,
    int& score, int& highScore, int& lives, int& nextLifeScore, int& wave,
    float& waveTimer, bool& newHighScore, int& selectedShip)
{
    return GameContext{
        renderer, sheet, rng, saveData, titleMenu,
        bgAsteroids, ship, bullets, asteroids,
        ufo, ufoBullets, ufoSpawnTimer,
        score, highScore, lives, nextLifeScore, wave, waveTimer, newHighScore, selectedShip
    };
}

AsteroidsGame::AsteroidsGame()
    : Engine::Application("Asteroids", W, H)
    , m_rng(std::random_device{}())
    , m_titleMenu({"PLAY", "EXIT"}, 3.f)
    , m_ctx(makeContext(m_renderer, m_sheet, m_rng, m_saveData, m_titleMenu,
                        m_bgAsteroids, m_ship, m_bullets, m_asteroids,
                        m_ufo, m_ufoBullets, m_state.ufoSpawnTimer,
                        m_state.score, m_highScore, m_state.lives, m_state.nextLifeScore, m_state.wave,
                        m_state.waveTimer, m_state.newHighScore, m_selectedShip))
    , m_titleScreen(m_ctx)
    , m_shipSelectScreen(m_ctx)
    , m_playingScreen(m_ctx)
    , m_gameOverScreen(m_ctx)
{
    loadAllConfigs();

    auto texture = std::make_shared<Engine::Texture>("games/asteroids/assets/asteroids-arcade.png");
    m_sheet      = std::make_shared<Engine::SpriteSheet>(texture, 16, 16);

    m_saveData  = Engine::SaveData::load("asteroids");
    m_highScore = m_saveData.getInt("high_score", 0);

    m_titleScreen.onEnter();
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
        case Screen::GameOver:   m_playingScreen.render();    break;  // game world stays visible
        default: break;
    }
}

void AsteroidsGame::onOverlayRender() {
    if (m_screen == Screen::GameOver)
        m_gameOverScreen.render();
}

// ---- transitions ------------------------------------------------------------

void AsteroidsGame::transitionTo(Screen next) {
    if (next == Screen::Quit) { quit(); return; }

    if (next == Screen::Playing && m_screen == Screen::ShipSelect)
        m_playingScreen.onEnter();

    if (next == Screen::ShipSelect && m_screen == Screen::GameOver)
        resetForRestart();

    m_screen = next;
}

void AsteroidsGame::resetForRestart() {
    m_asteroids.clear();
    m_bullets.clear();
    m_ufo.reset();
    m_ufoBullets.clear();
    m_ship.reset();
    Engine::Particles::clear();
    m_state = GameState{};
}

