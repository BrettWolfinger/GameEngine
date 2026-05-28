#pragma once
#include <engine/core/Application.h>
#include <engine/core/Input.h>
#include <engine/renderer/Renderer2D.h>
#include <engine/renderer/Texture.h>
#include <engine/renderer/SpriteSheet.h>
#include <engine/tilemap/Tilemap.h>
#include <engine/tilemap/TilemapRenderer.h>
#include "GameTypes.h"
#include "Pacman.h"
#include "Ghost.h"
#include "PacmanConfig.h"
#include "PacmanHudConfig.h"
#include <memory>
#include <optional>
#include <vector>

class PacmanGame : public Engine::Application {
public:
    PacmanGame();

protected:
    void onInit()            override;
    void onUpdate(float dt)  override;
    void onRender()          override;
    Engine::Renderer2D* getRenderer() override { return &m_renderer; }

private:
    enum class GameState { Playing, Dying, LevelClear, GameOver };

    void buildDotCache();
    void tryEatDot();
    void updateModeTimer(float dt);
    void triggerFrightened();
    void updateFrightenedTimer(float dt);
    void checkGhostCollision();
    void checkGhostRelease();
    void startDeathSequence();
    void handleDyingState(float dt);
    void respawnAfterDeath();
    void startLevelClear();
    void startNextLevel();
    void restartGame();
    void resetModeSchedule();
    void loadHighScore();
    void saveHighScore();
    void updateHighScore();
    void renderWalls();
    void renderDots();
    void renderGhosts();
    void renderHUD();
    void renderOverlay();
    void renderLives();
    void renderDevHUD();

    Engine::Renderer2D                    m_renderer;
    std::shared_ptr<Engine::Texture>      m_wallTex;
    std::shared_ptr<Engine::SpriteSheet>  m_wallSheet;
    std::shared_ptr<Engine::Texture>      m_itemsTex;
    std::shared_ptr<Engine::SpriteSheet>  m_itemsSheet;
    std::shared_ptr<Engine::Texture>      m_pacTex;
    std::shared_ptr<Engine::SpriteSheet>  m_pacSheet;
    std::shared_ptr<Engine::Texture>      m_ghostTex;
    std::shared_ptr<Engine::SpriteSheet>  m_ghostSheet;
    std::shared_ptr<Engine::Texture>      m_facesTex;
    std::shared_ptr<Engine::SpriteSheet>  m_facesSheet;
    Engine::Tilemap::Map                  m_map;
    const Engine::Tilemap::TileLayer*     m_dotsLayer = nullptr; // cached after map load
    const Engine::Tilemap::TileLayer*     m_doorLayer = nullptr; // ghost house door layer
    std::vector<CellType>                 m_dots;
    int                                   m_score     = 0;
    int                                   m_dotsEaten = 0; // counts dots/pellets eaten for ghost release

    std::optional<Pacman>                 m_pacman;  // constructed after map load
    std::vector<Ghost>                    m_ghosts;  // constructed after map load

    // Scatter/chase mode cycling (Level 1 schedule)
    float     m_modeTimer   = 7.f;   // seconds until next mode switch
    int       m_modePhase   = 0;     // index into schedule; even=Scatter, odd=Chase
    GhostMode m_currentMode = GhostMode::Scatter;

    // Frightened mode
    float     m_frightenedTimer        = 0.f;
    int       m_ghostsEatenThisPellet  = 0;

    // Lives, death sequence, and level state
    int       m_lives            = kStartLives;
    GameState m_gameState        = GameState::Playing;
    float     m_deathPauseTimer  = 0.f;
    float     m_levelClearTimer  = 0.f;
    int       m_dotsRemaining    = 0;

    // Scoring
    int       m_highScore        = 0;

    PacmanConfig    m_config;
    PacmanHudConfig m_hudConfig;
};
