#pragma once
#include <engine/core/Application.h>
#include <engine/renderer/Renderer2D.h>
#include <engine/renderer/SpriteAnimator.h>
#include <memory>
#include <optional>

class FroggerGame : public Engine::Application {
public:
    FroggerGame();

protected:
    void onUpdate(float dt) override;
    void onRender()         override;

private:
    struct Frog {
        int   col      = 6;    // starting column (center)
        int   row      = 13;   // starting row (bottom)
        bool  hopping  = false;
        float hopTimer = 0.f;  // counts down while hop animation plays
    };

    Engine::Renderer2D                      m_renderer;
    std::shared_ptr<Engine::SpriteSheet>    m_sheet;
    std::optional<Engine::SpriteAnimator>   m_animator;
    Frog                                    m_frog;
};
