#include "AsteroidsGame.h"
#include "AsteroidsConfig.h"
#include <engine/renderer/Texture.h>
#include <engine/core/Input.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <algorithm>
#include <random>
#include <cmath>

AsteroidsGame::AsteroidsGame()
    : Engine::Application("Asteroids", W, H)
{
    auto texture = std::make_shared<Engine::Texture>("games/asteroids/assets/asteroids-arcade.png");
    m_sheet      = std::make_shared<Engine::SpriteSheet>(texture, 16, 16);
    m_ship.emplace(m_sheet);

    // Spawn 4 large asteroids, each at least 150px from screen center.
    std::mt19937 rng(42);
    std::uniform_real_distribution<float> angleDist(0.f, glm::two_pi<float>());
    std::uniform_real_distribution<float> speedDist(40.f, 80.f);
    std::uniform_real_distribution<float> rotDist(0.5f, 1.5f);
    std::uniform_int_distribution<int>    rotSignDist(0, 1);

    const glm::vec2 center(W * 0.5f, H * 0.5f);
    constexpr float MIN_DIST  = 150.f;
    constexpr int   COUNT     = 4;

    // Fixed spawn positions arranged in a ring so they're naturally spread out
    // and guaranteed to be at least MIN_DIST from center.
    for (int i = 0; i < COUNT; ++i) {
        // Evenly space the base directions, then jitter by up to 30° to avoid
        // a perfectly symmetric look.
        const float baseAngle   = (glm::two_pi<float>() / COUNT) * i;
        const float jitter      = std::uniform_real_distribution<float>(
                                      -glm::pi<float>() / 6.f,
                                       glm::pi<float>() / 6.f)(rng);
        const float spawnAngle  = baseAngle + jitter;
        const float spawnRadius = MIN_DIST + std::uniform_real_distribution<float>(0.f, 120.f)(rng);

        glm::vec2 pos = center + glm::vec2(std::cos(spawnAngle), std::sin(spawnAngle)) * spawnRadius;

        // Clamp to screen bounds (leaving a cell-width margin so the asteroid
        // is fully visible at spawn).
        pos.x = std::clamp(pos.x, 32.f, static_cast<float>(W) - 32.f);
        pos.y = std::clamp(pos.y, 32.f, static_cast<float>(H) - 32.f);

        const float velAngle = angleDist(rng);
        const float speed    = speedDist(rng);
        glm::vec2   vel(std::cos(velAngle) * speed, std::sin(velAngle) * speed);

        const float rotMag  = rotDist(rng);
        const float rotSign = rotSignDist(rng) ? 1.f : -1.f;

        m_asteroids.push_back(Asteroid::makeLarge(pos, vel, rotMag * rotSign));
    }
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
        b.uv       = m_sheet->getFrameUVs(128);  // bullet, 1×1
        b.tex      = &m_sheet->texture();
        m_bullets.push_back(b);
    }

    for (auto& b : m_bullets)
        b.update(dt);

    m_bullets.erase(
        std::remove_if(m_bullets.begin(), m_bullets.end(),
                       [](const Bullet& b) { return !b.isAlive(); }),
        m_bullets.end());

    for (auto& a : m_asteroids)
        a.update(dt, W, H);
}

void AsteroidsGame::onRender() {
    m_renderer.beginScene(W, H);

    for (const auto& a : m_asteroids)
        a.render(m_renderer, *m_sheet);

    m_ship->render(m_renderer);

    for (const auto& b : m_bullets)
        b.render(m_renderer);
}
