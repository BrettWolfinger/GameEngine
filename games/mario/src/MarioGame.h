#pragma once
#include "MarioConfig.h"
#include "Player.h"
#include <engine/core/Application.h>
#include <engine/core/Input.h>
#include <engine/renderer/Renderer2D.h>
#include <engine/renderer/Texture.h>
#include <engine/renderer/SpriteSheet.h>
#include <engine/tilemap/Tilemap.h>
#include <engine/tilemap/TilemapRenderer.h>
#include <memory>

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

    Engine::Renderer2D                   m_renderer;
    std::shared_ptr<Engine::Texture>     m_tilesetTex;
    std::shared_ptr<Engine::SpriteSheet> m_tilesetSheet;
    std::shared_ptr<Engine::Texture>     m_marioTex;
    std::shared_ptr<Engine::SpriteSheet> m_marioSheet;
    Engine::Tilemap::Map                 m_map;
    MarioConfig                          m_config;
    float                                m_cameraX = 0.f;
    std::unique_ptr<Player>              m_player;
};
