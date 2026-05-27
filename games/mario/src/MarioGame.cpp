#include "MarioGame.h"
#include "GameConstants.h"
#include <GLFW/glfw3.h>

MarioGame::MarioGame()
    : Engine::Application("Super Mario Bros", WIN_W, WIN_H)
{}

void MarioGame::onUpdate(float /*dt*/) {
    if (Engine::Input::isKeyPressed(GLFW_KEY_Q))
        quit();
}

void MarioGame::onRender() {
    m_renderer.beginScene(WIN_W, WIN_H);
}
