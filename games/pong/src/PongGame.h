#pragma once
#include <engine/core/Application.h>
#include <engine/renderer/Renderer2D.h>
#include "Paddle.h"
#include "Ball.h"

class PongGame : public Engine::Application {
public:
    PongGame();

protected:
    void onInit()           override;
    void onUpdate(float dt) override;
    void onRender()         override;

private:
    enum class GameState { Playing, WinScreen };

    void resetBall();
    void resetGame();

    Engine::Renderer2D m_renderer;
    Paddle m_left{};
    Paddle m_right{};
    Ball   m_ball{};
    int    m_scoreLeft  = 0;
    int    m_scoreRight = 0;
    bool   m_paused     = false;
    float  m_countdown  = 0.f;

    GameState m_state   = GameState::Playing;
    int       m_winner  = 0;   // 1 = left player, 2 = right player
    float     m_winFlash = 0.f; // drives blinking prompt on win screen
};
