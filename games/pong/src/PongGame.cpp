#include "PongGame.h"
#include <engine/renderer/SegmentFont.h>
#include <engine/core/Input.h>
#include <engine/Engine.h>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------
static constexpr int   W            = 800;
static constexpr int   H            = 600;
static constexpr float PADDLE_W     = 14.f;
static constexpr float PADDLE_H     = 80.f;
static constexpr float PADDLE_SPEED = 400.f;
static constexpr float BALL_SIZE    = 12.f;
static constexpr float BALL_SPEED   = 300.f;
static constexpr float MAX_SPEED    = 650.f;
static constexpr int   WIN_SCORE    = 7;

// Single fixed AI parameters — slightly tougher than the old Easy setting
static constexpr float AI_SPEED         = 200.f; // px/s
static constexpr int   AI_REACTION_DELAY = 10;   // frames of lag in history buffer
static constexpr float AI_MAX_ERROR     = 12.f;  // ±px target error per bounce
static constexpr float AI_DEAD_ZONE     = 4.f;   // px — ignore tiny deviations

// ---------------------------------------------------------------------------
PongGame::PongGame() : Engine::Application("Pong", W, H) {}


// ---------------------------------------------------------------------------
void PongGame::resetBall() {
    float vx = (rand() % 2 == 0) ?  BALL_SPEED : -BALL_SPEED;
    float vy = (rand() % 2 == 0) ?  200.f      : -200.f;
    m_ball = { W/2.f - BALL_SIZE/2.f, H/2.f - BALL_SIZE/2.f, vx, vy, BALL_SIZE };
    m_countdown = 3.f;
    // Clear history so AI doesn't use stale data from previous rally
    m_historyHead  = 0;
    m_historyCount = 0;
    newAITargetOffset();
}

void PongGame::resetGame() {
    m_scoreLeft  = 0;
    m_scoreRight = 0;
    m_winner     = 0;
    m_winFlash   = 0.f;
    m_paused     = false;
    m_state      = GameState::ModeSelect;  // return to mode select between games
    m_left  = { 20.f,                 H/2.f - PADDLE_H/2.f, PADDLE_W, PADDLE_H, PADDLE_SPEED };
    m_right = { W - 20.f - PADDLE_W,  H/2.f - PADDLE_H/2.f, PADDLE_W, PADDLE_H, PADDLE_SPEED };
    m_historyHead  = 0;
    m_historyCount = 0;
}

// ---------------------------------------------------------------------------
// AI helpers
// ---------------------------------------------------------------------------
void PongGame::newAITargetOffset() {
    float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX); // [0,1]
    m_aiTargetOffset = (r * 2.f - 1.f) * AI_MAX_ERROR;
}

void PongGame::updateAI(float dt) {
    // Push current ball Y into circular history buffer
    m_ballYHistory[m_historyHead] = m_ball.y + m_ball.size * 0.5f;
    m_historyHead = (m_historyHead + 1) % HISTORY_SIZE;
    if (m_historyCount < HISTORY_SIZE) ++m_historyCount;

    // Read ball Y from AI_REACTION_DELAY frames ago
    int delay   = std::min(AI_REACTION_DELAY, m_historyCount - 1);
    if (delay < 0) delay = 0;
    int readIdx = (m_historyHead - 1 - delay + HISTORY_SIZE * 2) % HISTORY_SIZE;
    float targetBallY = m_ballYHistory[readIdx];

    // Target paddle center with per-bounce error baked in
    float targetPaddleCenter  = targetBallY + m_aiTargetOffset;
    float currentPaddleCenter = m_right.y + m_right.height * 0.5f;
    float diff = targetPaddleCenter - currentPaddleCenter;

    // Dead zone — don't move if already close enough (avoids jitter)
    if (std::abs(diff) < AI_DEAD_ZONE) return;

    // Move toward target at capped speed
    float move = std::copysign(std::min(std::abs(diff), AI_SPEED * dt), diff);
    m_right.y += move;
    m_right.y  = std::clamp(m_right.y, 0.f, H - m_right.height);
}

// ---------------------------------------------------------------------------
// onUpdate — thin dispatcher
// ---------------------------------------------------------------------------
#ifdef ENABLE_DEV_KEYS
void PongGame::updateDevKeys() {
    // F1: mode select  F2: start 2P game  F3: P1 wins  F4: P2 wins  F5: set scores to WIN_SCORE-1
    if (Engine::Input::isKeyPressed(GLFW_KEY_F1)) { resetGame(); return; }
    if (Engine::Input::isKeyPressed(GLFW_KEY_F2)) {
        m_singlePlayer = false;
        m_left  = { 20.f,                H/2.f - PADDLE_H/2.f, PADDLE_W, PADDLE_H, PADDLE_SPEED };
        m_right = { W - 20.f - PADDLE_W, H/2.f - PADDLE_H/2.f, PADDLE_W, PADDLE_H, PADDLE_SPEED };
        m_state = GameState::Playing;
        resetBall();
        return;
    }
    if (Engine::Input::isKeyPressed(GLFW_KEY_F3)) { m_winner = 1; m_winFlash = 0.f; m_state = GameState::WinScreen; return; }
    if (Engine::Input::isKeyPressed(GLFW_KEY_F4)) { m_winner = 2; m_winFlash = 0.f; m_state = GameState::WinScreen; return; }
    if (Engine::Input::isKeyPressed(GLFW_KEY_F5)) { m_scoreLeft = WIN_SCORE - 1; m_scoreRight = WIN_SCORE - 1; }
}
#endif

void PongGame::onUpdate(float dt) {
    if (Engine::Input::isKeyPressed(GLFW_KEY_Q))
        quit();

#ifdef ENABLE_DEV_KEYS
    updateDevKeys();
#endif
    if (Engine::Input::isKeyPressed(GLFW_KEY_ESCAPE) && m_state != GameState::ModeSelect) {
        resetGame();
        return;
    }

    switch (m_state) {
        case GameState::ModeSelect:  updateModeSelect();    break;
        case GameState::WinScreen:   updateWinScreen(dt);   break;
        case GameState::Playing:
            if (Engine::Input::isKeyPressed(GLFW_KEY_P))
                m_paused = !m_paused;
            if (!m_paused) {
                updatePaddles(dt);
                updateBall(dt);
            }
            break;
    }
}

void PongGame::updateModeSelect() {
    if (Engine::Input::isKeyPressed(GLFW_KEY_ESCAPE))
        quit();

    bool startGame = false;
    if (Engine::Input::isKeyPressed(GLFW_KEY_ENTER) ||
        Engine::Input::isKeyPressed(GLFW_KEY_KP_ENTER) || // numpad Enter

        Engine::Input::isKeyPressed(GLFW_KEY_1)) {
        m_singlePlayer = true;
        startGame = true;
    } else if (Engine::Input::isKeyPressed(GLFW_KEY_SPACE) ||
               Engine::Input::isKeyPressed(GLFW_KEY_2)) {
        m_singlePlayer = false;
        startGame = true;
    }

    if (startGame) {
        m_left  = { 20.f,                H/2.f - PADDLE_H/2.f, PADDLE_W, PADDLE_H, PADDLE_SPEED };
        m_right = { W - 20.f - PADDLE_W, H/2.f - PADDLE_H/2.f, PADDLE_W, PADDLE_H, PADDLE_SPEED };
        m_state = GameState::Playing;
        resetBall();
    }
}

void PongGame::updateWinScreen(float dt) {
    m_winFlash += dt;
    if (Engine::Input::isKeyPressed(GLFW_KEY_R))
        resetGame();
}

void PongGame::updatePaddles(float dt) {
    if (Engine::Input::isKeyDown(GLFW_KEY_W))
        m_left.y -= m_left.speed * dt;
    if (Engine::Input::isKeyDown(GLFW_KEY_S))
        m_left.y += m_left.speed * dt;
    m_left.y = std::clamp(m_left.y, 0.f, H - m_left.height);

    if (m_singlePlayer) {
        if (m_countdown <= 0.f)
            updateAI(dt);
    } else {
        if (Engine::Input::isKeyDown(GLFW_KEY_UP))
            m_right.y -= m_right.speed * dt;
        if (Engine::Input::isKeyDown(GLFW_KEY_DOWN))
            m_right.y += m_right.speed * dt;
        m_right.y = std::clamp(m_right.y, 0.f, H - m_right.height);
    }
}

void PongGame::updateBall(float dt) {
    if (m_countdown > 0.f) {
        m_countdown -= dt;
        return;
    }

    m_ball.x += m_ball.vx * dt;
    m_ball.y += m_ball.vy * dt;

    if (m_ball.y < 0.f)             { m_ball.y = 0.f;             m_ball.vy = -m_ball.vy; Engine::playTone(240.f, 0.04f); }
    if (m_ball.y + m_ball.size > H) { m_ball.y = H - m_ball.size; m_ball.vy = -m_ball.vy; Engine::playTone(240.f, 0.04f); }

    if (m_ball.x <= m_left.x + m_left.width &&
        m_ball.x + m_ball.size >= m_left.x  &&
        m_ball.y + m_ball.size >= m_left.y  &&
        m_ball.y <= m_left.y + m_left.height)
    {
        m_ball.x  = m_left.x + m_left.width;
        m_ball.vx = std::min(std::abs(m_ball.vx) * 1.05f, MAX_SPEED);
        Engine::playTone(480.f, 0.05f);
    }

    bool rightHit =
        m_ball.x + m_ball.size >= m_right.x &&
        m_ball.x <= m_right.x + m_right.width &&
        m_ball.y + m_ball.size >= m_right.y   &&
        m_ball.y <= m_right.y + m_right.height;

    if (rightHit) {
        m_ball.x  = m_right.x - m_ball.size;
        m_ball.vx = -std::min(std::abs(m_ball.vx) * 1.05f, MAX_SPEED);
        if (m_singlePlayer) newAITargetOffset();
        Engine::playTone(480.f, 0.05f);
    }

    if (m_ball.x + m_ball.size < 0.f) {
        ++m_scoreRight;
        Engine::playTone(120.f, 0.3f);
        if (m_scoreRight >= WIN_SCORE) { m_winner = 2; m_state = GameState::WinScreen; }
        else resetBall();
    } else if (m_ball.x > W) {
        ++m_scoreLeft;
        Engine::playTone(120.f, 0.3f);
        if (m_scoreLeft >= WIN_SCORE) { m_winner = 1; m_state = GameState::WinScreen; }
        else resetBall();
    }
}

// ---------------------------------------------------------------------------
// onRender — thin dispatcher
// ---------------------------------------------------------------------------
void PongGame::onRender() {
    m_renderer.beginScene(W, H);
    switch (m_state) {
        case GameState::ModeSelect: renderModeSelect(); break;
        case GameState::WinScreen:  renderWinScreen();  break;
        case GameState::Playing:    renderPlaying();    break;
    }
}

void PongGame::renderModeSelect() {
    const glm::vec4 white { 1.f,  1.f,  1.f,  1.f };
    const glm::vec4 gray  { 0.4f, 0.4f, 0.4f, 1.f };
    const glm::vec4 dim   { 0.08f, 0.08f, 0.08f, 1.f };

    m_renderer.drawRect(0.f, 0.f, W, H, dim);

    float s  = 14.f;
    float cy = H * 0.5f - s * 2.5f;

    Engine::SegmentFont::drawStringCentered(m_renderer, "1P", W * 0.25f, cy, s, white);
    Engine::SegmentFont::drawStringCentered(m_renderer, "2P", W * 0.75f, cy, s, white);

    m_renderer.drawRect(W * 0.5f - 2.f, cy - 20.f, 4.f, s * 5.f + 40.f, gray);

    float hs    = 5.f;
    float hintY = cy + s * 5.f + 16.f;
    Engine::SegmentFont::drawStringCentered(m_renderer, "SPACE", W * 0.25f, hintY, hs, gray);
    Engine::SegmentFont::drawStringCentered(m_renderer, "ENTER", W * 0.75f, hintY, hs, gray);
}

void PongGame::renderWinScreen() {
    const glm::vec4 white { 1.f,   1.f,   1.f,  1.f };
    const glm::vec4 gray  { 0.4f,  0.4f,  0.4f, 1.f };
    const glm::vec4 gold  { 1.f,   0.85f, 0.1f, 1.f };
    const glm::vec4 dim   { 0.08f, 0.08f, 0.08f, 1.f };

    m_renderer.drawRect(0.f, 0.f, W, H, dim);

    Engine::SegmentFont::drawStringCentered(m_renderer, m_scoreLeft,  W * 0.25f, 30.f, 10.f, gray);
    Engine::SegmentFont::drawStringCentered(m_renderer, m_scoreRight, W * 0.75f, 30.f, 10.f, gray);

    float digitScale = 28.f;
    float digitCX    = W * 0.5f;
    float digitY     = H * 0.5f - 90.f;
    float barW       = 8.f;
    float barH       = digitScale * 5.f + 8.f;
    float bracketGap = digitScale * 3.f * 0.5f + 20.f;
    m_renderer.drawRect(digitCX - bracketGap - barW, digitY - 4.f, barW, barH, gold);
    m_renderer.drawRect(digitCX + bracketGap,        digitY - 4.f, barW, barH, gold);
    Engine::SegmentFont::drawStringCentered(m_renderer, m_winner, digitCX, digitY, digitScale, gold);

    float labelY = digitY - 24.f;
    float lbW = 12.f, lbH = 6.f, lbGap = 6.f;
    float labelX = digitCX - (3.f * lbW + 2.f * lbGap) * 0.5f;
    for (int i = 0; i < 3; ++i)
        m_renderer.drawRect(labelX + i * (lbW + lbGap), labelY, lbW, lbH, gold);

    if (std::fmod(m_winFlash, 1.0f) < 0.6f) {
        float promptY = digitY + barH + 20.f;
        float promptW = 60.f, promptH = 8.f;
        m_renderer.drawRect(digitCX - promptW - 10.f, promptY, promptW, promptH, white);
        m_renderer.drawRect(digitCX + 10.f,           promptY, promptW, promptH, white);
        m_renderer.drawRect(digitCX - promptW - 10.f, promptY, 8.f, promptH * 3.f, white);
        m_renderer.drawRect(digitCX + promptW + 2.f,  promptY, 8.f, promptH * 3.f, white);
    }
}

void PongGame::renderPlaying() {
    const glm::vec4 white { 1.f,  1.f,  1.f,  1.f };
    const glm::vec4 gray  { 0.4f, 0.4f, 0.4f, 1.f };

    for (int y = 0; y < H; y += 30)
        m_renderer.drawRect(W/2.f - 2.f, (float)y, 4.f, 16.f, gray);

    m_renderer.drawRect(m_left.x,  m_left.y,  m_left.width,  m_left.height,  white);
    m_renderer.drawRect(m_right.x, m_right.y, m_right.width, m_right.height, white);
    m_renderer.drawRect(m_ball.x,  m_ball.y,  m_ball.size,   m_ball.size,    white);

    Engine::SegmentFont::drawStringCentered(m_renderer, m_scoreLeft,  W * 0.25f, 30.f, 10.f, white);
    Engine::SegmentFont::drawStringCentered(m_renderer, m_scoreRight, W * 0.75f, 30.f, 10.f, white);

    if (m_countdown > 0.f)
        Engine::SegmentFont::drawStringCentered(m_renderer, static_cast<int>(m_countdown) + 1, W * 0.5f, H * 0.5f - 25.f, 14.f, white);
}
