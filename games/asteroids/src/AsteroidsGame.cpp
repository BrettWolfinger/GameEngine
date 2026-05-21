#include "AsteroidsGame.h"
#include "AsteroidsConfig.h"
#include <engine/renderer/Texture.h>
#include <engine/core/Input.h>
#include <GLFW/glfw3.h>

AsteroidsGame::AsteroidsGame()
    : Engine::Application("Asteroids", W, H)
{
    auto texture = std::make_shared<Engine::Texture>("games/asteroids/assets/asteroids-arcade.png");
    m_sheet      = std::make_shared<Engine::SpriteSheet>(texture, 8, 8);
    m_ship.emplace(m_sheet);
}

void AsteroidsGame::onUpdate(float dt) {
    if (Engine::Input::isKeyPressed(GLFW_KEY_Q))
        quit();

    m_ship->update(dt, W, H);
}

void AsteroidsGame::onRender() {
    m_renderer.beginScene(W, H);
    m_ship->render(m_renderer);
}
