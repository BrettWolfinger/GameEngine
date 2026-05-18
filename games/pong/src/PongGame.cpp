#include "PongGame.h"
#include <engine/core/Input.h>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>

// ---------------------------------------------------------------------------
// 7-segment digit renderer
// Segments: a=top b=top-right c=bottom-right d=bottom e=bottom-left f=top-left g=middle
// ---------------------------------------------------------------------------
static void drawDigit(Engine::Renderer2D& r, int d, float x, float y, float s, const glm::vec4& col) {
    static const int segs[10] = { 63, 6, 91, 79, 102, 109, 125, 7, 127, 111 };
    int mask = (d >= 0 && d <= 9) ? segs[d] : 0;
    if (mask &  1) r.drawRect(x,      y,      3*s,  s,   col); // a top
    if (mask &  2) r.drawRect(x+2*s,  y,       s,  3*s,  col); // b top-right
    if (mask &  4) r.drawRect(x+2*s,  y+2*s,   s,  3*s,  col); // c bottom-right
    if (mask &  8) r.drawRect(x,      y+4*s,  3*s,  s,   col); // d bottom
    if (mask & 16) r.drawRect(x,      y+2*s,   s,  3*s,  col); // e bottom-left
    if (mask & 32) r.drawRect(x,      y,        s,  3*s,  col); // f top-left
    if (mask & 64) r.drawRect(x,      y+2*s,  3*s,  s,   col); // g middle
}

// "P" = segments a, b, e, f, g  (top, top-right, bottom-left, top-left, middle)
static void drawP(Engine::Renderer2D& r, float x, float y, float s, const glm::vec4& col) {
    r.drawRect(x,       y,      3*s,  s,   col); // a top
    r.drawRect(x+2*s,   y,       s,  3*s,  col); // b top-right
    r.drawRect(x,       y+2*s,   s,  3*s,  col); // e bottom-left
    r.drawRect(x,       y,        s,  3*s,  col); // f top-left
    r.drawRect(x,       y+2*s,  3*s,  s,   col); // g middle
}

// Draws "NP" (e.g. "1P" or "2P") centered on cx
static void drawNP(Engine::Renderer2D& r, int n, float cx, float y, float s, const glm::vec4& col) {
    float digitW = 3*s, gap = s;
    float totalW = digitW + gap + digitW;
    float x = cx - totalW / 2.f;
    drawDigit(r, n, x, y, s, col);
    drawP(r, x + digitW + gap, y, s, col);
}

static void drawNumber(Engine::Renderer2D& r, int n, float cx, float y, float s, const glm::vec4& col) {
    float digitW = 3*s;
    float gap    = s;
    float totalW = (n >= 10) ? digitW + gap + digitW : digitW;
    float x      = cx - totalW / 2.f;
    if (n >= 10) {
        drawDigit(r, n / 10, x, y, s, col);
        x += digitW + gap;
    }
    drawDigit(r, n % 10, x, y, s, col);
}

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

void PongGame::onInit() {
    m_left  = { 20.f,                 H/2.f - PADDLE_H/2.f, PADDLE_W, PADDLE_H, PADDLE_SPEED };
    m_right = { W - 20.f - PADDLE_W,  H/2.f - PADDLE_H/2.f, PADDLE_W, PADDLE_H, PADDLE_SPEED };
    // Don't call resetBall here — we start on ModeSelect screen
    m_historyHead  = 0;
    m_historyCount = 0;
}

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
// onUpdate
// ---------------------------------------------------------------------------
void PongGame::onUpdate(float dt) {
    // Quit is always active
    if (Engine::Input::isKeyPressed(GLFW_KEY_Q) ||
        Engine::Input::isKeyPressed(GLFW_KEY_ESCAPE))
        quit();

    // --- Mode select screen ---
    if (m_state == GameState::ModeSelect) {
        // Enter = single-player
        if (Engine::Input::isKeyPressed(GLFW_KEY_ENTER) ||
            Engine::Input::isKeyPressed(GLFW_KEY_KP_ENTER)) {
            m_singlePlayer = true;
            m_state = GameState::Playing;
            resetBall();
            return;
        }
        // Space = two-player
        if (Engine::Input::isKeyPressed(GLFW_KEY_SPACE)) {
            m_singlePlayer = false;
            m_state = GameState::Playing;
            resetBall();
            return;
        }
        return;
    }

    // --- Win screen ---
    if (m_state == GameState::WinScreen) {
        m_winFlash += dt;
        if (Engine::Input::isKeyPressed(GLFW_KEY_R))
            resetGame();
        return;
    }

    // --- Playing ---
    if (Engine::Input::isKeyPressed(GLFW_KEY_P))
        m_paused = !m_paused;

    if (m_paused) return;

    // Left paddle — always player-controlled
    if (Engine::Input::isKeyDown(GLFW_KEY_W))
        m_left.y -= m_left.speed * dt;
    if (Engine::Input::isKeyDown(GLFW_KEY_S))
        m_left.y += m_left.speed * dt;
    m_left.y = std::clamp(m_left.y, 0.f, H - m_left.height);

    // Right paddle — AI or player
    if (m_singlePlayer) {
        if (m_countdown <= 0.f) {
            // AI only moves once the ball is in play
            updateAI(dt);
        }
    } else {
        if (Engine::Input::isKeyDown(GLFW_KEY_UP))
            m_right.y -= m_right.speed * dt;
        if (Engine::Input::isKeyDown(GLFW_KEY_DOWN))
            m_right.y += m_right.speed * dt;
        m_right.y = std::clamp(m_right.y, 0.f, H - m_right.height);
    }

    if (m_countdown > 0.f) {
        m_countdown -= dt;
        return;
    }

    // --- Ball movement ---
    m_ball.x += m_ball.vx * dt;
    m_ball.y += m_ball.vy * dt;

    // Top/bottom walls
    if (m_ball.y < 0.f)                 { m_ball.y = 0.f;               m_ball.vy = -m_ball.vy; }
    if (m_ball.y + m_ball.size > H)     { m_ball.y = H - m_ball.size;   m_ball.vy = -m_ball.vy; }

    // Left paddle collision (AABB)
    if (m_ball.x <= m_left.x + m_left.width &&
        m_ball.x + m_ball.size >= m_left.x  &&
        m_ball.y + m_ball.size >= m_left.y  &&
        m_ball.y <= m_left.y + m_left.height)
    {
        m_ball.x  = m_left.x + m_left.width;
        m_ball.vx = std::min(std::abs(m_ball.vx) * 1.05f, MAX_SPEED);
    }

    // Right paddle collision — recalculate AI target offset on each bounce
    bool rightHit =
        m_ball.x + m_ball.size >= m_right.x &&
        m_ball.x <= m_right.x + m_right.width &&
        m_ball.y + m_ball.size >= m_right.y   &&
        m_ball.y <= m_right.y + m_right.height;

    if (rightHit) {
        m_ball.x  = m_right.x - m_ball.size;
        m_ball.vx = -std::min(std::abs(m_ball.vx) * 1.05f, MAX_SPEED);
        if (m_singlePlayer) newAITargetOffset(); // new error for next approach
    }

    // Scoring
    if (m_ball.x + m_ball.size < 0.f) {
        ++m_scoreRight;
        if (m_scoreRight >= WIN_SCORE) {
            m_winner = 2;
            m_state  = GameState::WinScreen;
        } else {
            resetBall();
        }
    } else if (m_ball.x > W) {
        ++m_scoreLeft;
        if (m_scoreLeft >= WIN_SCORE) {
            m_winner = 1;
            m_state  = GameState::WinScreen;
        } else {
            resetBall();
        }
    }
}

// ---------------------------------------------------------------------------
// onRender
// ---------------------------------------------------------------------------
void PongGame::onRender() {
    m_renderer.beginScene(W, H);

    const glm::vec4 white { 1.f,   1.f,   1.f,  1.f };
    const glm::vec4 gray  { 0.4f,  0.4f,  0.4f, 1.f };
    const glm::vec4 gold  { 1.f,   0.85f, 0.1f, 1.f };
    const glm::vec4 dim   { 0.08f, 0.08f, 0.08f, 1.f };
    const glm::vec4 cyan  { 0.3f,  0.9f,  0.9f, 1.f };

    // -----------------------------------------------------------------------
    // Mode select screen
    // -----------------------------------------------------------------------
    if (m_state == GameState::ModeSelect) {
        m_renderer.drawRect(0.f, 0.f, W, H, dim);

        float s  = 14.f;  // segment scale — each glyph is 42×70px
        float cy = H * 0.5f - s * 2.5f; // vertically centered

        // "1P" on the left quarter
        drawNP(m_renderer, 1, W * 0.25f, cy, s, white);

        // "2P" on the right quarter
        drawNP(m_renderer, 2, W * 0.75f, cy, s, white);

        // Vertical divider
        m_renderer.drawRect(W * 0.5f - 2.f, cy - 20.f, 4.f, s * 5.f + 40.f, gray);

        // Key hints — thin wide bar below each label
        float hintY = cy + s * 5.f + 16.f;
        m_renderer.drawRect(W * 0.25f - 36.f, hintY, 72.f, 8.f, gray);  // SPACE
        m_renderer.drawRect(W * 0.75f - 36.f, hintY, 72.f, 8.f, white); // ENTER

        return;
    }

    // -----------------------------------------------------------------------
    // Win screen
    // -----------------------------------------------------------------------
    if (m_state == GameState::WinScreen) {
        m_renderer.drawRect(0.f, 0.f, W, H, dim);

        drawNumber(m_renderer, m_scoreLeft,  W * 0.25f, 30.f, 10.f, gray);
        drawNumber(m_renderer, m_scoreRight, W * 0.75f, 30.f, 10.f, gray);

        float digitScale = 28.f;
        float digitCX    = W * 0.5f;
        float digitY     = H * 0.5f - 90.f;
        float barW       = 8.f;
        float barH       = digitScale * 5.f + 8.f;
        float bracketGap = digitScale * 3.f * 0.5f + 20.f;
        m_renderer.drawRect(digitCX - bracketGap - barW, digitY - 4.f, barW, barH, gold);
        m_renderer.drawRect(digitCX + bracketGap,        digitY - 4.f, barW, barH, gold);
        drawNumber(m_renderer, m_winner, digitCX, digitY, digitScale, gold);

        float labelY = digitY - 24.f;
        float lbW = 12.f, lbH = 6.f, lbGap = 6.f;
        float labelX = digitCX - (3.f * lbW + 2.f * lbGap) * 0.5f;
        for (int i = 0; i < 3; ++i)
            m_renderer.drawRect(labelX + i * (lbW + lbGap), labelY, lbW, lbH, gold);

        bool flashVisible = std::fmod(m_winFlash, 1.0f) < 0.6f;
        if (flashVisible) {
            float promptY = digitY + barH + 20.f;
            float promptW = 60.f, promptH = 8.f;
            m_renderer.drawRect(digitCX - promptW - 10.f, promptY, promptW, promptH, white);
            m_renderer.drawRect(digitCX + 10.f,           promptY, promptW, promptH, white);
            m_renderer.drawRect(digitCX - promptW - 10.f, promptY,             8.f, promptH * 3.f, white);
            m_renderer.drawRect(digitCX + promptW + 2.f,  promptY,             8.f, promptH * 3.f, white);
        }

        return;
    }

    // -----------------------------------------------------------------------
    // Normal play rendering
    // -----------------------------------------------------------------------

    // Dashed center line
    for (int y = 0; y < H; y += 30)
        m_renderer.drawRect(W/2.f - 2.f, (float)y, 4.f, 16.f, gray);

    m_renderer.drawRect(m_left.x,  m_left.y,  m_left.width,  m_left.height,  white);
    m_renderer.drawRect(m_right.x, m_right.y, m_right.width, m_right.height, white);
    m_renderer.drawRect(m_ball.x,  m_ball.y,  m_ball.size,   m_ball.size,    white);

    // Scores
    drawNumber(m_renderer, m_scoreLeft,  W * 0.25f, 30.f, 10.f, white);
    drawNumber(m_renderer, m_scoreRight, W * 0.75f, 30.f, 10.f, white);

    // Countdown
    if (m_countdown > 0.f) {
        int count = static_cast<int>(m_countdown) + 1;
        drawNumber(m_renderer, count, W * 0.5f, H * 0.5f - 25.f, 14.f, white);
    }
}
