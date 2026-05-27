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
    void buildDotCache();
    void tryEatDot();
    void updateModeTimer(float dt);
    void renderWalls();
    void renderDots();
    void renderGhosts();
    void renderHUD();
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
    Engine::Tilemap::Map                  m_map;
    std::vector<CellType>                 m_dots;
    int                                   m_score = 0;

    std::optional<Pacman>                 m_pacman;  // constructed after map load
    std::vector<Ghost>                    m_ghosts;  // constructed after map load

    // Scatter/chase mode cycling (Level 1 schedule)
    float     m_modeTimer   = 7.f;   // seconds until next mode switch
    int       m_modePhase   = 0;     // index into schedule; even=Scatter, odd=Chase
    GhostMode m_currentMode = GhostMode::Scatter;
};
