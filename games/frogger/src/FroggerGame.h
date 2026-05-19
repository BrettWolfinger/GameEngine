#pragma once
#include <engine/core/Application.h>
#include <engine/renderer/Renderer2D.h>
#include <engine/renderer/SpriteSheet.h>
#include "Frog.h"
#include <memory>
#include <optional>

class FroggerGame : public Engine::Application {
public:
    FroggerGame();

protected:
    void onUpdate(float dt) override;
    void onRender()         override;

    void renderBackground();

private:
    Engine::Renderer2D                   m_renderer;
    std::shared_ptr<Engine::SpriteSheet> m_sheet;
    std::optional<Frog>                  m_frog;
};
