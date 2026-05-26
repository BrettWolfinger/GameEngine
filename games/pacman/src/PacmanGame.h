#pragma once
#include <engine/core/Application.h>
#include <engine/core/Input.h>
#include <engine/renderer/Renderer2D.h>
#include <engine/renderer/Texture.h>
#include <engine/renderer/SpriteSheet.h>
#include "MapTypes.h"
#include <memory>

class PacmanGame : public Engine::Application {
public:
    PacmanGame();

protected:
    void onInit()            override;
    void onUpdate(float dt)  override;
    void onRender()          override;
    Engine::Renderer2D* getRenderer() override { return &m_renderer; }

private:
    void renderWalls();
    void renderDots();

    Engine::Renderer2D                   m_renderer;
    std::shared_ptr<Engine::Texture>     m_wallTex;
    std::shared_ptr<Engine::SpriteSheet> m_wallSheet;
    std::shared_ptr<Engine::Texture>     m_itemsTex;
    std::shared_ptr<Engine::SpriteSheet> m_itemsSheet;
    MapData                              m_map;
};
