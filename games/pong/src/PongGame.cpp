#include "PongGame.h"
#include <engine/core/Input.h>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cstdlib>
#include <iostream>

static constexpr int   W            = 800;
static constexpr int   H            = 600;
static constexpr float PADDLE_W     = 14.f;
static constexpr float PADDLE_H     = 80.f;
static constexpr float PADDLE_SPEED = 400.f;
static constexpr float BALL_SIZE    = 12.f;
static constexpr float BALL_SPEED   = 300.f;
static constexpr float MAX_SPEED    = 650.f;

PongGame::PongGame() : Engine::Application("Pong", W, H) {}

void PongGame::onInit() {
    m_left  = { 20.f,                   H/2.f - PADDLE_H/2.f, PADDLE_W, PADDLE_H, PADDLE_SPEED };
    m_right = { W - 20.f - PADDLE_W,    H/2.f - PADDLE_H/2.f, PADDLE_W, PADDLE_H, PADDLE_SPEED };
    resetBall();
}

void PongGame::resetBall() {
    float vx = (rand() % 2 == 0) ?  BALL_SPEED : -BALL_SPEED;
    float vy = (rand() % 2 == 0) ?  200.f      : -200.f;
    m_ball = { W/2.f - BALL_SIZE/2.f, H/2.f - BALL_SIZE/2.f, vx, vy, BALL_SIZE };
}

void PongGame::onUpdate(float dt) {
    // --- Input ---
    if (Engine::Input::isKeyDown(GLFW_KEY_W))
        m_left.y -= m_left.speed * dt;
    if (Engine::Input::isKeyDown(GLFW_KEY_S))
        m_left.y += m_left.speed * dt;
    if (Engine::Input::isKeyDown(GLFW_KEY_UP))
        m_right.y -= m_right.speed * dt;
    if (Engine::Input::isKeyDown(GLFW_KEY_DOWN))
        m_right.y += m_right.speed * dt;

    m_left.y  = std::clamp(m_left.y,  0.f, H - m_left.height);
    m_right.y = std::clamp(m_right.y, 0.f, H - m_right.height);

    // --- Ball movement ---
    m_ball.x += m_ball.vx * dt;
    m_ball.y += m_ball.vy * dt;

    // Top/bottom walls
    if (m_ball.y < 0.f)                    { m_ball.y = 0.f;                    m_ball.vy = -m_ball.vy; }
    if (m_ball.y + m_ball.size > H)        { m_ball.y = H - m_ball.size;        m_ball.vy = -m_ball.vy; }

    // Left paddle collision (AABB)
    if (m_ball.x <= m_left.x + m_left.width &&
        m_ball.x + m_ball.size >= m_left.x  &&
        m_ball.y + m_ball.size >= m_left.y  &&
        m_ball.y <= m_left.y + m_left.height)
    {
        m_ball.x  = m_left.x + m_left.width;
        m_ball.vx = std::min( std::abs(m_ball.vx) * 1.05f, MAX_SPEED);
    }

    // Right paddle collision
    if (m_ball.x + m_ball.size >= m_right.x &&
        m_ball.x <= m_right.x + m_right.width &&
        m_ball.y + m_ball.size >= m_right.y   &&
        m_ball.y <= m_right.y + m_right.height)
    {
        m_ball.x  = m_right.x - m_ball.size;
        m_ball.vx = -std::min(std::abs(m_ball.vx) * 1.05f, MAX_SPEED);
    }

    // Scoring
    if (m_ball.x + m_ball.size < 0.f) {
        ++m_scoreRight;
        std::cout << "Score  Left " << m_scoreLeft << "  Right " << m_scoreRight << "\n";
        resetBall();
    } else if (m_ball.x > W) {
        ++m_scoreLeft;
        std::cout << "Score  Left " << m_scoreLeft << "  Right " << m_scoreRight << "\n";
        resetBall();
    }

    if (Engine::Input::isKeyPressed(GLFW_KEY_ESCAPE))
        quit();
}

void PongGame::onRender() {
    m_renderer.beginScene(W, H);

    const glm::vec4 white  { 1.f, 1.f, 1.f, 1.f };
    const glm::vec4 gray   { 0.4f, 0.4f, 0.4f, 1.f };

    // Dashed center line
    for (int y = 0; y < H; y += 30)
        m_renderer.drawRect(W/2.f - 2.f, (float)y, 4.f, 16.f, gray);

    m_renderer.drawRect(m_left.x,  m_left.y,  m_left.width,  m_left.height,  white);
    m_renderer.drawRect(m_right.x, m_right.y, m_right.width, m_right.height, white);
    m_renderer.drawRect(m_ball.x,  m_ball.y,  m_ball.size,   m_ball.size,    white);
}
