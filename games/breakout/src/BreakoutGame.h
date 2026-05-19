#pragma once
#include <engine/core/Application.h>
#include <engine/renderer/Renderer2D.h>
#include <array>
#include <glm/glm.hpp>

struct Brick { float x, y, w, h; bool alive; glm::vec4 color; int points; };

class BreakoutGame : public Engine::Application {
public:
    BreakoutGame();

protected:
    void onUpdate(float dt) override;
    void onRender()         override;

private:
    static constexpr int ROWS = 8;
    static constexpr int COLS = 14;

    enum class GameState { TitleScreen, Playing, GameOver, WinScreen };

    void resetGame();
    void resetBall();
    void initBricks();

    void updateTitleScreen();
    void updatePlaying(float dt);
    void updateGameOver();
    void updateWinScreen();

#ifdef ENABLE_DEV_KEYS
    void updateDevKeys();
#endif

    void renderTitleScreen();
    void renderPlaying();
    void renderGameOver();
    void renderWinScreen();

    Engine::Renderer2D m_renderer;

    struct Paddle { float x, y, w, h; };
    struct Ball    { float x, y, size, vx, vy; };

    Paddle m_paddle{};
    Ball   m_ball{};

    std::array<Brick, ROWS * COLS> m_bricks{};

    int   m_score    = 0;
    int   m_lives    = 3;
    bool  m_ballHeld = true;
    int   m_bricksDestroyed = 0;

    GameState m_state = GameState::TitleScreen;
};
