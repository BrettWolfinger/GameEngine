#pragma once
#include <engine/core/Application.h>
#include <engine/core/SaveData.h>
#include <engine/renderer/Renderer2D.h>
#include <engine/renderer/SpriteSheet.h>
#include <engine/renderer/Texture.h>
#include <engine/ui/Menu.h>
#include "GameContext.h"
#include "TitleScreen.h"
#include "ShipSelectScreen.h"
#include "PlayingScreen.h"
#include "GameOverScreen.h"
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

protected:
    void preStep(float dt)  override;
    void onUpdate(float dt) override;
    void onRender()         override;

private:
    // ---- resources ----
    Engine::Renderer2D                     m_renderer;
    std::shared_ptr<Engine::SpriteSheet>   m_sheet;
    std::mt19937                           m_rng;
    Engine::SaveData                       m_saveData;
    Engine::Menu                           m_titleMenu;

    // ---- game objects ----
    std::vector<std::unique_ptr<Asteroid>> m_bgAsteroids;
    std::optional<Ship>                    m_ship;
    std::vector<std::unique_ptr<Bullet>>   m_bullets;
    std::vector<std::unique_ptr<Asteroid>> m_asteroids;

    // ---- game state ----
    int   m_score        = 0;
    int   m_highScore    = 0;
    int   m_lives        = 3;
    int   m_wave         = 1;
    float m_waveTimer    = -1.f;
    bool  m_newHighScore = false;
    int   m_selectedShip = 0;

    // ---- screen routing ----
    Screen          m_screen = Screen::Title;
    GameContext     m_ctx;
    TitleScreen     m_titleScreen;
    ShipSelectScreen m_shipSelectScreen;
    PlayingScreen   m_playingScreen;
    GameOverScreen  m_gameOverScreen;

    void transitionTo(Screen next);
    void resetForRestart();
};
