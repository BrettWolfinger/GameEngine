#include "BreakoutGame.h"
#include <engine/renderer/SegmentFont.h>
#include <engine/renderer/PixelFont.h>
#include <engine/core/Input.h>
#include <engine/audio/AudioManager.h>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>

static constexpr int   W              = 800;
static constexpr int   H              = 600;

static constexpr float PADDLE_W       = 100.f;
static constexpr float PADDLE_H       = 14.f;
static constexpr float PADDLE_Y       = 550.f;
static constexpr float PADDLE_SPEED   = 500.f;

static constexpr float PADDLE_W_NARROW = 50.f;

static constexpr float BALL_SIZE      = 10.f;
static constexpr float BALL_SPEED     = 280.f;
static constexpr float MAX_SPEED      = 600.f;
static constexpr float MAX_VX_FROM_PADDLE = 300.f;

static constexpr float BRICK_W        = 52.f;
static constexpr float BRICK_H        = 16.f;
static constexpr float BRICK_GAP      = 2.f;
static constexpr float BRICK_TOP      = 60.f;

static constexpr int   BRICKS_PER_SPEED_STEP = 5;
static constexpr float SPEED_MULTIPLIER      = 1.08f;

// ---------------------------------------------------------------------------
BreakoutGame::BreakoutGame() : Engine::Application("Breakout", W, H) {
    m_saveData  = Engine::SaveData::load("breakout");
    m_highScore = m_saveData.getInt("high_score", 0);
}

// ---------------------------------------------------------------------------
void BreakoutGame::initBricks() {
    // Total brick grid width: COLS bricks + (COLS-1) gaps
    float totalW = COLS * BRICK_W + (COLS - 1) * BRICK_GAP;
    float startX = (W - totalW) * 0.5f;

    for (int row = 0; row < ROWS; ++row) {
        glm::vec4 color;
        int points;
        if (row <= 1) {
            color  = { 0.55f, 0.55f, 0.55f, 1.f };
            points = 1;
        } else if (row <= 3) {
            color  = { 0.2f, 0.85f, 0.3f, 1.f };
            points = 3;
        } else if (row <= 5) {
            color  = { 1.f, 0.65f, 0.1f, 1.f };
            points = 5;
        } else {
            color  = { 0.95f, 0.2f, 0.2f, 1.f };
            points = 7;
        }

        for (int col = 0; col < COLS; ++col) {
            int idx = row * COLS + col;
            // Rows are stored bottom=0, so row 7 is at the top visually.
            // We draw top rows higher on screen: invert row index for Y.
            float bx = startX + col * (BRICK_W + BRICK_GAP);
            float by = BRICK_TOP + (ROWS - 1 - row) * (BRICK_H + BRICK_GAP);
            m_bricks[idx] = { bx, by, BRICK_W, BRICK_H, true, color, points };
        }
    }
}

void BreakoutGame::resetBall() {
    float paddleCX = m_paddle.x + m_paddle.w * 0.5f;
    m_ball = { paddleCX - BALL_SIZE * 0.5f, PADDLE_Y - BALL_SIZE - 1.f, BALL_SIZE, 0.f, 0.f };
    m_ballHeld = true;
}

void BreakoutGame::resetGame() {
    m_score           = 0;
    m_lives           = 3;
    m_bricksDestroyed = 0;
    m_ceilingHit      = false;
    // m_highScore intentionally not reset — persists across sessions
    m_paddle          = { W * 0.5f - PADDLE_W * 0.5f, PADDLE_Y, PADDLE_W, PADDLE_H };
    initBricks();
    resetBall();
    m_state = GameState::Playing;
}

#ifdef ENABLE_DEV_KEYS
void BreakoutGame::updateDevKeys() {
    // F1: title  F2: start game  F3: game over  F4: win screen  F5: clear all bricks  F6: lose a life
    if (Engine::Input::isKeyPressed(GLFW_KEY_F1)) { m_state = GameState::TitleScreen; return; }
    if (Engine::Input::isKeyPressed(GLFW_KEY_F2)) { resetGame(); return; }
    if (Engine::Input::isKeyPressed(GLFW_KEY_F3)) { m_state = GameState::GameOver;    return; }
    if (Engine::Input::isKeyPressed(GLFW_KEY_F4)) { m_state = GameState::WinScreen;   return; }
    if (Engine::Input::isKeyPressed(GLFW_KEY_F5)) {
        for (auto& b : m_bricks) b.alive = false;
        m_state = GameState::WinScreen;
        return;
    }
    if (Engine::Input::isKeyPressed(GLFW_KEY_F6)) {
        --m_lives;
        if (m_lives <= 0) m_state = GameState::GameOver;
        else resetBall();
    }
}
#endif

// ---------------------------------------------------------------------------
// onUpdate
// ---------------------------------------------------------------------------
void BreakoutGame::onUpdate(float dt) {
    if (Engine::Input::isKeyPressed(GLFW_KEY_Q))
        quit();

#ifdef ENABLE_DEV_KEYS
    updateDevKeys();
#endif

    switch (m_state) {
        case GameState::TitleScreen: updateTitleScreen(); break;
        case GameState::Playing:     updatePlaying(dt);  break;
        case GameState::GameOver:    updateGameOver();    break;
        case GameState::WinScreen:   updateWinScreen();   break;
    }
}

void BreakoutGame::updateTitleScreen() {
    if (Engine::Input::isKeyPressed(GLFW_KEY_SPACE))
        resetGame();
}

void BreakoutGame::updatePlaying(float dt) {
    // Paddle movement
    if (Engine::Input::isKeyDown(GLFW_KEY_LEFT))
        m_paddle.x -= PADDLE_SPEED * dt;
    if (Engine::Input::isKeyDown(GLFW_KEY_RIGHT))
        m_paddle.x += PADDLE_SPEED * dt;
    m_paddle.x = std::clamp(m_paddle.x, 0.f, W - m_paddle.w);

    // Ball follows paddle while held
    if (m_ballHeld) {
        m_ball.x = m_paddle.x + m_paddle.w * 0.5f - m_ball.size * 0.5f;
        if (Engine::Input::isKeyPressed(GLFW_KEY_SPACE)) {
            // Random upward angle between -60 and +60 degrees from straight up
            float angle = ((float)(rand() % 121) - 60.f) * 3.14159265f / 180.f;
            m_ball.vx = BALL_SPEED * std::sin(angle);
            m_ball.vy = -BALL_SPEED * std::cos(angle);
            m_ballHeld = false;
        }
        return;
    }

    m_ball.x += m_ball.vx * dt;
    m_ball.y += m_ball.vy * dt;

    // Wall bounces (left, right, ceiling)
    if (m_ball.x < 0.f) {
        m_ball.x  = 0.f;
        m_ball.vx = std::abs(m_ball.vx);
        Engine::AudioManager::playTone(240.f, 0.04f);
    }
    if (m_ball.x + m_ball.size > W) {
        m_ball.x  = W - m_ball.size;
        m_ball.vx = -std::abs(m_ball.vx);
        Engine::AudioManager::playTone(240.f, 0.04f);
    }
    if (m_ball.y < 0.f) {
        m_ball.y  = 0.f;
        m_ball.vy = std::abs(m_ball.vy);
        Engine::AudioManager::playTone(240.f, 0.04f);
        if (!m_ceilingHit) {
            m_ceilingHit    = true;
            float cx        = m_paddle.x + m_paddle.w * 0.5f;
            m_paddle.w      = PADDLE_W_NARROW;
            m_paddle.x      = std::clamp(cx - m_paddle.w * 0.5f, 0.f, W - m_paddle.w);
        }
    }

    // Ball lost off bottom
    if (m_ball.y > H) {
        --m_lives;
        Engine::AudioManager::playTone(120.f, 0.3f);
        if (m_lives <= 0) {
            if (m_score > m_highScore) {
                m_highScore = m_score;
                m_saveData.setInt("high_score", m_highScore);
                m_saveData.save();
            }
            m_state = GameState::GameOver;
        } else {
            resetBall();
        }
        return;
    }

    // Paddle collision
    float ballCX   = m_ball.x + m_ball.size * 0.5f;
    float ballCY   = m_ball.y + m_ball.size * 0.5f;
    float paddleCX = m_paddle.x + m_paddle.w * 0.5f;

    bool hitPaddle = m_ball.x + m_ball.size >= m_paddle.x &&
                     m_ball.x               <= m_paddle.x + m_paddle.w &&
                     m_ball.y + m_ball.size >= m_paddle.y &&
                     m_ball.y               <= m_paddle.y + m_paddle.h &&
                     m_ball.vy > 0.f; // only when traveling downward

    if (hitPaddle) {
        float normalize = (ballCX - paddleCX) / (m_paddle.w * 0.5f);
        normalize = std::clamp(normalize, -1.f, 1.f);

        float speed = std::sqrt(m_ball.vx * m_ball.vx + m_ball.vy * m_ball.vy);
        m_ball.vx   = normalize * MAX_VX_FROM_PADDLE;
        m_ball.vy   = -std::abs(m_ball.vy);

        // Restore original speed magnitude
        float newSpeed = std::sqrt(m_ball.vx * m_ball.vx + m_ball.vy * m_ball.vy);
        if (newSpeed > 0.f) {
            float scale = speed / newSpeed;
            m_ball.vx *= scale;
            m_ball.vy *= scale;
        }

        // Push ball above paddle to avoid repeated hits
        m_ball.y = m_paddle.y - m_ball.size;
        Engine::AudioManager::playTone(480.f, 0.05f);
    }

    // Brick collision — find deepest-overlap brick, resolve only that one
    int    bestIdx     = -1;
    float  bestOverlap = 0.f;

    for (int i = 0; i < ROWS * COLS; ++i) {
        if (!m_bricks[i].alive) continue;
        const Brick& b = m_bricks[i];

        float overlapX = std::min(m_ball.x + m_ball.size, b.x + b.w) - std::max(m_ball.x, b.x);
        float overlapY = std::min(m_ball.y + m_ball.size, b.y + b.h) - std::max(m_ball.y, b.y);

        if (overlapX <= 0.f || overlapY <= 0.f) continue;

        float total = overlapX + overlapY;
        if (total > bestOverlap) {
            bestOverlap = total;
            bestIdx     = i;
        }
    }

    if (bestIdx >= 0) {
        Brick& b = m_bricks[bestIdx];

        float overlapX = std::min(m_ball.x + m_ball.size, b.x + b.w) - std::max(m_ball.x, b.x);
        float overlapY = std::min(m_ball.y + m_ball.size, b.y + b.h) - std::max(m_ball.y, b.y);

        if (overlapX < overlapY) {
            // Shallower on X axis — hit a side face
            m_ball.vx = -m_ball.vx;
        } else {
            // Shallower on Y axis — hit top or bottom face
            m_ball.vy = -m_ball.vy;
        }

        // Determine which row this brick belongs to for pitch calculation
        int row = bestIdx / COLS;
        Engine::AudioManager::playTone(600.f + row * 40.f, 0.04f);

        b.alive = false;
        m_score += b.points;
        ++m_bricksDestroyed;

        // Speed increase every BRICKS_PER_SPEED_STEP bricks
        if (m_bricksDestroyed % BRICKS_PER_SPEED_STEP == 0) {
            float speed = std::sqrt(m_ball.vx * m_ball.vx + m_ball.vy * m_ball.vy);
            speed = std::min(speed * SPEED_MULTIPLIER, MAX_SPEED);
            float cur = std::sqrt(m_ball.vx * m_ball.vx + m_ball.vy * m_ball.vy);
            if (cur > 0.f) {
                float scale = speed / cur;
                m_ball.vx *= scale;
                m_ball.vy *= scale;
            }
        }

        // Check win condition
        bool anyAlive = false;
        for (const auto& brick : m_bricks) {
            if (brick.alive) { anyAlive = true; break; }
        }
        if (!anyAlive) {
            if (m_score > m_highScore) {
                m_highScore = m_score;
                m_saveData.setInt("high_score", m_highScore);
                m_saveData.save();
            }
            m_state = GameState::WinScreen;
        }
    }
}

void BreakoutGame::updateGameOver() {
    if (Engine::Input::isKeyPressed(GLFW_KEY_R))
        resetGame();
}

void BreakoutGame::updateWinScreen() {
    if (Engine::Input::isKeyPressed(GLFW_KEY_R))
        resetGame();
}

// ---------------------------------------------------------------------------
// onRender
// ---------------------------------------------------------------------------
void BreakoutGame::onRender() {
    m_renderer.beginScene(W, H);
    switch (m_state) {
        case GameState::TitleScreen: renderTitleScreen(); break;
        case GameState::Playing:     renderPlaying();     break;
        case GameState::GameOver:    renderGameOver();    break;
        case GameState::WinScreen:   renderWinScreen();   break;
    }
}

void BreakoutGame::renderTitleScreen() {
    const glm::vec4 white { 1.f,   1.f,   1.f,  1.f };
    const glm::vec4 gray  { 0.4f,  0.4f,  0.4f, 1.f };
    const glm::vec4 dim   { 0.08f, 0.08f, 0.08f, 1.f };

    m_renderer.drawRect(0.f, 0.f, W, H, dim);

    float titleScale = 8.f;
    float titleY     = H * 0.35f;
    Engine::PixelFont::drawStringCentered(m_renderer, "BREAKOUT", W * 0.5f, titleY, titleScale, white);

    float promptScale = 4.f;
    float promptY     = titleY + 7.f * titleScale + 40.f;
    Engine::PixelFont::drawStringCentered(m_renderer, "PRESS SPACE", W * 0.5f, promptY, promptScale, gray);
}

void BreakoutGame::renderPlaying() {
    const glm::vec4 white { 1.f,  1.f,  1.f,  1.f };
    const glm::vec4 dim   { 0.08f, 0.08f, 0.08f, 1.f };

    m_renderer.drawRect(0.f, 0.f, W, H, dim);

    for (const auto& b : m_bricks) {
        if (b.alive)
            m_renderer.drawRect(b.x, b.y, b.w, b.h, b.color);
    }

    m_renderer.drawRect(m_paddle.x, m_paddle.y, m_paddle.w, m_paddle.h, white);
    m_renderer.drawRect(m_ball.x,   m_ball.y,   m_ball.size, m_ball.size, white);

    const glm::vec4 gold { 1.f, 0.85f, 0.1f, 1.f };
    float hudScale = 8.f;
    Engine::SegmentFont::drawStringCentered(m_renderer, m_score,     W * 0.25f, 10.f, hudScale, white);
    Engine::SegmentFont::drawStringCentered(m_renderer, m_highScore, W * 0.5f,  10.f, hudScale, gold);
    Engine::SegmentFont::drawStringCentered(m_renderer, m_lives,     W * 0.75f, 10.f, hudScale, white);
}

void BreakoutGame::renderGameOver() {
    const glm::vec4 white { 1.f,   1.f,   1.f,  1.f };
    const glm::vec4 red   { 0.95f, 0.2f,  0.2f, 1.f };
    const glm::vec4 gray  { 0.4f,  0.4f,  0.4f, 1.f };
    const glm::vec4 dim   { 0.08f, 0.08f, 0.08f, 1.f };

    m_renderer.drawRect(0.f, 0.f, W, H, dim);

    float titleScale = 6.f;
    float titleY     = H * 0.3f;
    Engine::PixelFont::drawStringCentered(m_renderer, "GAME OVER", W * 0.5f, titleY, titleScale, red);

    float scoreScale = 8.f;
    float scoreY     = titleY + 7.f * titleScale + 24.f;
    Engine::SegmentFont::drawStringCentered(m_renderer, m_score, W * 0.5f, scoreY, scoreScale, white);

    float promptScale = 4.f;
    float promptY     = scoreY + 5.f * scoreScale + 24.f;
    Engine::PixelFont::drawStringCentered(m_renderer, "R TO RESTART", W * 0.5f, promptY, promptScale, gray);
}

void BreakoutGame::renderWinScreen() {
    const glm::vec4 white  { 1.f,   1.f,   1.f,  1.f };
    const glm::vec4 gold   { 1.f,   0.85f, 0.1f, 1.f };
    const glm::vec4 gray   { 0.4f,  0.4f,  0.4f, 1.f };
    const glm::vec4 dim    { 0.08f, 0.08f, 0.08f, 1.f };

    m_renderer.drawRect(0.f, 0.f, W, H, dim);

    float titleScale = 6.f;
    float titleY     = H * 0.3f;
    Engine::PixelFont::drawStringCentered(m_renderer, "YOU WIN", W * 0.5f, titleY, titleScale, gold);

    float scoreScale = 8.f;
    float scoreY     = titleY + 7.f * titleScale + 24.f;
    Engine::SegmentFont::drawStringCentered(m_renderer, m_score, W * 0.5f, scoreY, scoreScale, white);

    float promptScale = 4.f;
    float promptY     = scoreY + 5.f * scoreScale + 24.f;
    Engine::PixelFont::drawStringCentered(m_renderer, "R TO RESTART", W * 0.5f, promptY, promptScale, gray);
}
