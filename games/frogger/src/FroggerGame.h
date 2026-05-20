#pragma once
#include <engine/core/Application.h>
#include <engine/renderer/Renderer2D.h>
#include <engine/renderer/SpriteSheet.h>
#include "Frog.h"
#include "Vehicle.h"
#include "Platform.h"
#include <memory>
#include <optional>
#include <vector>
#include <string_view>

class FroggerGame : public Engine::Application {
public:
    FroggerGame();

protected:
    void onUpdate(float dt) override;
    void onRender()         override;

private:
    enum class GameState { Playing, GameOver, Win };

    void renderBackground();
    void renderHUD();
    void renderEndScreen(std::string_view title, const glm::vec4& titleColor);
    void die();
    void restartGame();

    Engine::Renderer2D                   m_renderer;
    std::shared_ptr<Engine::SpriteSheet> m_sheet;
    std::optional<Frog>                  m_frog;
    std::vector<Vehicle>                 m_vehicles;
    std::vector<Platform>                m_platforms;
    bool                                 m_filledSlots[HOME_SLOT_COUNT] = {};
    bool                                 m_allHomesFilled = false;
    int                                  m_lives = 3;
    GameState                            m_state = GameState::Playing;
    float                                m_deathX     = 0.f;
    float                                m_deathY     = 0.f;
    float                                m_deathTimer = 0.f;
};
