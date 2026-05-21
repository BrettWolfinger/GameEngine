#pragma once
#include <engine/core/Application.h>
#include <engine/renderer/Renderer2D.h>
#include <engine/renderer/SpriteSheet.h>
#include <engine/renderer/Texture.h>
#include "Ship.h"
#include <memory>
#include <optional>

class AsteroidsGame : public Engine::Application {
public:
    AsteroidsGame();

protected:
    void onUpdate(float dt) override;
    void onRender()         override;

private:
    Engine::Renderer2D                   m_renderer;
    std::shared_ptr<Engine::SpriteSheet> m_sheet;
    std::optional<Ship>                  m_ship;
};
