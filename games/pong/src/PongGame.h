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
    void resetBall();

    Engine::Renderer2D m_renderer;
    Paddle m_left{};
    Paddle m_right{};
    Ball   m_ball{};
    int    m_scoreLeft  = 0;
    int    m_scoreRight = 0;
};
