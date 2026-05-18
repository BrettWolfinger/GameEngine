#pragma once
#include <engine/core/Application.h>
#include <engine/renderer/Renderer2D.h>
#include "Paddle.h"
#include "Ball.h"

class PongGame : public Engine::Application {
public:
    PongGame();

protected:
    void onUpdate(float dt) override;
    void onRender()         override;

private:
    enum class GameState { ModeSelect, Playing, WinScreen };

    void resetBall();
    void resetGame();

    // onUpdate sub-steps
    void updateModeSelect();
    void updateWinScreen(float dt);
    void updatePaddles(float dt);
    void updateBall(float dt);

    // onRender sub-steps
    void renderModeSelect();
    void renderWinScreen();
    void renderPlaying();

    // AI helpers
    void updateAI(float dt);
    void newAITargetOffset();

    Engine::Renderer2D m_renderer;
    Paddle m_left{};
    Paddle m_right{};
    Ball   m_ball{};
    int    m_scoreLeft  = 0;
    int    m_scoreRight = 0;
    bool   m_paused     = false;
    float  m_countdown  = 0.f;

    GameState m_state    = GameState::ModeSelect;
    int       m_winner   = 0;   // 1 = left, 2 = right
    float     m_winFlash = 0.f; // drives blinking prompt on win screen

    // Single-player / AI state
    bool  m_singlePlayer   = false;
    float m_aiTargetOffset = 0.f; // per-bounce random error (pixels)

    // Circular buffer of recent ball Y positions for reaction delay
    static constexpr int HISTORY_SIZE = 20;
    float m_ballYHistory[HISTORY_SIZE] = {};
    int   m_historyHead  = 0; // index of next write slot
    int   m_historyCount = 0; // how many valid entries
};
