#pragma once
#include "MarioConfig.h"
#include "GoombaConfig.h"
#include "Player.h"
#include "Goomba.h"
#include <engine/core/Application.h>
#include <engine/core/Input.h>
#include <engine/renderer/Renderer2D.h>
#include <engine/renderer/Texture.h>
#include <engine/renderer/SpriteSheet.h>
#include <engine/tilemap/Tilemap.h>
#include <engine/tilemap/TilemapRenderer.h>
#include <engine/tilemap/TilemapCollider.h>
#include <memory>
#include <optional>
#include <vector>

class MarioGame : public Engine::Application {
public:
    MarioGame();

protected:
    void onInit()           override;
    void onUpdate(float dt) override;
    void onRender()         override;
    Engine::Renderer2D* getRenderer() override { return &m_renderer; }

private:
    void renderBackground();
    void renderTerrain();
    void renderPlayer();
    void renderEnemies();
    void checkEnemyCollisions();

    Engine::Renderer2D                   m_renderer;
    std::shared_ptr<Engine::Texture>     m_tilesetTex;
    std::shared_ptr<Engine::SpriteSheet> m_tilesetSheet;
    std::shared_ptr<Engine::Texture>     m_marioTex;
    std::shared_ptr<Engine::SpriteSheet> m_marioSheet;
    std::shared_ptr<Engine::Texture>     m_enemiesTex;
    std::shared_ptr<Engine::SpriteSheet> m_enemiesSheet;
    Engine::Tilemap::Map                 m_map;
    MarioConfig                              m_config;
    GoombaConfig                             m_goombaConfig;
    std::optional<Engine::Tilemap::Collider>   m_collider;
    float                                    m_cameraX  = 0.f;
    float                                    m_startX   = 0.f;
    float                                    m_startY   = 0.f;
    std::unique_ptr<Player>                  m_player;
    std::vector<Goomba>                      m_goombas;
};
