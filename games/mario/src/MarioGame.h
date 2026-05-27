#pragma once
#include <engine/core/Application.h>
#include <engine/core/Input.h>
#include <engine/renderer/Renderer2D.h>
#include <engine/renderer/Texture.h>
#include <engine/renderer/SpriteSheet.h>
#include "MapLoader.h"
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

    Engine::Renderer2D                   m_renderer;
    std::shared_ptr<Engine::Texture>     m_tilesetTex;
    std::shared_ptr<Engine::SpriteSheet> m_tilesetSheet;
    MarioMap                             m_map;
    float                                m_marioX  = 0.f;
    float                                m_cameraX = 0.f;
};
