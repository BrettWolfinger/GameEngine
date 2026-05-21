#include "AsteroidsGame.h"
#include "AsteroidsConfig.h"
#include <engine/renderer/Texture.h>
#include <engine/core/Input.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <algorithm>

AsteroidsGame::AsteroidsGame()
    : Engine::Application("Asteroids", W, H)
{
    auto texture = std::make_shared<Engine::Texture>("games/asteroids/assets/asteroids-arcade.png");
    m_sheet      = std::make_shared<Engine::SpriteSheet>(texture, 16, 16);
    m_ship.emplace(m_sheet);
}

void AsteroidsGame::onUpdate(float dt) {
    if (Engine::Input::isKeyPressed(GLFW_KEY_Q))
        quit();

    m_ship->update(dt, W, H);

    if (m_ship->tryShoot() && (int)m_bullets.size() < MAX_BULLETS) {
        const float angle = m_ship->angle();
        glm::vec2 forward = { glm::sin(angle), -glm::cos(angle) };
        glm::vec2 nose    = m_ship->pos() + forward * (Ship::RENDER_SIZE * 0.5f);

        Bullet b;
        b.pos      = nose;
        b.vel      = forward * Bullet::SPEED;
        b.lifetime = Bullet::LIFETIME;
        b.uv       = m_sheet->getFrameUVs(128);  // bullet, 1x1
        b.tex      = &m_sheet->texture();
        m_bullets.push_back(b);
    }

    for (auto& b : m_bullets)
        b.update(dt);

    m_bullets.erase(
        std::remove_if(m_bullets.begin(), m_bullets.end(),
                       [](const Bullet& b) { return !b.isAlive(); }),
        m_bullets.end());
}

void AsteroidsGame::onRender() {
    m_renderer.beginScene(W, H);
    m_ship->render(m_renderer);
    for (const auto& b : m_bullets)
        b.render(m_renderer);
}
