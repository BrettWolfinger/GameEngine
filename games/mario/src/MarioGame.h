#pragma once
#include <engine/core/Application.h>
#include <engine/core/Input.h>
#include <engine/renderer/Renderer2D.h>

class MarioGame : public Engine::Application {
public:
    MarioGame();

protected:
    void onUpdate(float dt) override;
    void onRender()         override;
    Engine::Renderer2D* getRenderer() override { return &m_renderer; }

private:
    Engine::Renderer2D m_renderer;
};
