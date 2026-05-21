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

    std::mt19937 rng(42);
    std::uniform_real_distribution<float> angleDist(0.f, glm::two_pi<float>());
    std::uniform_real_distribution<float> speedDist(40.f, 80.f);
    std::uniform_real_distribution<float> rotDist(0.5f, 1.5f);
    std::uniform_int_distribution<int>    rotSignDist(0, 1);

    const glm::vec2 center(W * 0.5f, H * 0.5f);
    constexpr float MIN_DIST = 150.f;
    constexpr int   COUNT    = 4;

    for (int i = 0; i < COUNT; ++i) {
        const float baseAngle   = (glm::two_pi<float>() / COUNT) * i;
        const float jitter      = std::uniform_real_distribution<float>(
                                      -glm::pi<float>() / 6.f,
                                       glm::pi<float>() / 6.f)(rng);
        const float spawnAngle  = baseAngle + jitter;
        const float spawnRadius = MIN_DIST + std::uniform_real_distribution<float>(0.f, 120.f)(rng);

        glm::vec2 pos = center + glm::vec2(std::cos(spawnAngle), std::sin(spawnAngle)) * spawnRadius;
        pos.x = std::clamp(pos.x, 32.f, static_cast<float>(W) - 32.f);
        pos.y = std::clamp(pos.y, 32.f, static_cast<float>(H) - 32.f);

        const float velAngle = angleDist(rng);
        const float speed    = speedDist(rng);
        const float rotMag   = rotDist(rng);
        const float rotSign  = rotSignDist(rng) ? 1.f : -1.f;

        m_asteroids.push_back(Asteroid::makeLarge(
            pos,
            { std::cos(velAngle) * speed, std::sin(velAngle) * speed },
            rotMag * rotSign));
    }
}

void AsteroidsGame::onUpdate(float dt) {
    if (Engine::Input::isKeyPressed(GLFW_KEY_Q))
        quit();

    // Spawn fragments for asteroids killed by collision last tick.
    // Snapshot the count so newly-appended fragments are skipped this pass.
    const size_t n = m_asteroids.size();
    for (size_t i = 0; i < n; ++i) {
        Asteroid& a = *m_asteroids[i];
        if (!a.isDead()) continue;

        const float baseAngle = std::atan2(a.vel.y, a.vel.x);
        const float speed     = glm::length(a.vel) * 2.f;

        if (a.size == AsteroidSize::Large) {
            for (int j = 0; j < 4; ++j) {
                const float ang = baseAngle + glm::half_pi<float>() * j;
                m_asteroids.push_back(Asteroid::makeMedium(j, a.pos,
                    { std::cos(ang) * speed, std::sin(ang) * speed },
                    1.0f * (j % 2 == 0 ? 1.f : -1.f)));
            }
        } else if (a.size == AsteroidSize::Medium) {
            for (int j = 0; j < 4; ++j) {
                const float ang = baseAngle + glm::half_pi<float>() * j;
                m_asteroids.push_back(Asteroid::makeSmall(a.variant, j, a.pos,
                    { std::cos(ang) * speed, std::sin(ang) * speed },
                    1.5f * (j % 2 == 0 ? 1.f : -1.f)));
            }
        }
        // Small: destroyed, no fragments.
    }

    m_asteroids.erase(
        std::remove_if(m_asteroids.begin(), m_asteroids.end(),
                       [](const auto& a) { return a->isDead(); }),
        m_asteroids.end());

    // --- Normal per-tick updates ---
    m_ship->update(dt, W, H);

    if (auto shot = m_ship->tryShoot()) {
        m_bullets.push_back(std::make_unique<Bullet>(
            shot->pos, shot->direction * Bullet::SPEED,
            m_sheet->getFrameUVs(128), &m_sheet->texture()));
    }

    for (auto& b : m_bullets)
        b->update(dt);

    m_bullets.erase(
        std::remove_if(m_bullets.begin(), m_bullets.end(),
                       [](const auto& b) { return !b->isAlive(); }),
        m_bullets.end());

    for (auto& a : m_asteroids)
        a->update(dt, W, H);
}

void AsteroidsGame::onRender() {
    m_renderer.beginScene(W, H);

    for (const auto& a : m_asteroids)
        a->render(m_renderer, *m_sheet);

    m_ship->render(m_renderer);

    for (const auto& b : m_bullets)
        b->render(m_renderer);
}
