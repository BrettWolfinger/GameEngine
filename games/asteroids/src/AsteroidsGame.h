#pragma once
#include <engine/core/Application.h>
#include <engine/renderer/Renderer2D.h>
#include <engine/renderer/SpriteSheet.h>
#include <engine/renderer/Texture.h>
#include "Ship.h"
#include "Bullet.h"
#include "Asteroid.h"
#include <memory>
#include <optional>
#include <vector>

class AsteroidsGame : public Engine::Application {
public:
    AsteroidsGame();

protected:
    void preStep(float dt)  override;
    void onUpdate(float dt) override;
    void onRender()         override;

private:
    static constexpr int   STARTING_ASTEROID_COUNT                  = 4;
    static constexpr float STARTING_ASTEROID_MIN_DIST_FROM_PLAYER  = 150.f;
    static constexpr int   STARTING_LIVES                           = 3;

    Engine::Renderer2D                        m_renderer;
    std::shared_ptr<Engine::SpriteSheet>      m_sheet;
    std::optional<Ship>                       m_ship;
    std::vector<std::unique_ptr<Bullet>>      m_bullets;
    std::vector<std::unique_ptr<Asteroid>>    m_asteroids;
    int                                       m_lives    = STARTING_LIVES;
    bool                                      m_gameOver = false;

    void spawnInitialAsteroidRing();
};
