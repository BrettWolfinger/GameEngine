#include "AsteroidsGame.h"
#include "AsteroidsConfig.h"
#include <engine/renderer/Texture.h>
#include <engine/renderer/SegmentFont.h>
#include <engine/renderer/PixelFont.h>
#include <engine/core/Input.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <algorithm>
#include <random>
#include <cmath>

AsteroidsGame::AsteroidsGame()
    : Engine::Application("Asteroids", W, H)
    , m_rng(std::random_device{}())
{
    auto texture = std::make_shared<Engine::Texture>("games/asteroids/assets/asteroids-arcade.png");
    m_sheet      = std::make_shared<Engine::SpriteSheet>(texture, 16, 16);
    m_ship.emplace(m_sheet);

    spawnInitialAsteroidRing();
}

// ---- core loop --------------------------------------------------------------

void AsteroidsGame::preStep(float dt) {
    if (m_gameOver) return;

    m_ship->update(dt, W, H);

    for (auto& b : m_bullets)
        b->update(dt);

    for (auto& a : m_asteroids)
        a->update(dt, W, H);
}

void AsteroidsGame::onUpdate(float dt) {
    if (Engine::Input::isKeyPressed(GLFW_KEY_Q)) quit();
    if (m_gameOver) {
        if (Engine::Input::isKeyPressed(GLFW_KEY_R)) restartGame();
        return;
    }

#ifdef ENABLE_DEV_KEYS
    handleDevInput();
#endif

    if (m_ship->wasHit()) handleShipHit();

    spawnAsteroidFragments();
    removeDeadAsteroids();
    advanceWaveIfCleared(dt);
    tryFireBullet();
    removeDeadBullets();
}

void AsteroidsGame::onRender() {
    m_renderer.beginScene(W, H);

    for (const auto& a : m_asteroids)
        a->render(m_renderer, *m_sheet);

    m_ship->render(m_renderer);

    for (const auto& b : m_bullets)
        b->render(m_renderer);

    renderScore();
    renderLivesHUD();
    renderWaveAnnouncement();
    if (m_gameOver) renderGameOver();
}

// ---- onUpdate helpers -------------------------------------------------------

#ifdef ENABLE_DEV_KEYS
void AsteroidsGame::handleDevInput() {
    if (Engine::Input::isKeyPressed(GLFW_KEY_C))
        m_asteroids.clear();
}
#endif

void AsteroidsGame::handleShipHit() {
    m_bullets.clear();
    if (--m_lives > 0)
        m_ship->reset();
    else
        m_gameOver = true;
}

void AsteroidsGame::spawnAsteroidFragments() {
    const size_t n = m_asteroids.size();
    for (size_t i = 0; i < n; ++i) {
        if (!m_asteroids[i]->wasShot()) continue;
        for (auto& f : m_asteroids[i]->split())
            m_asteroids.push_back(std::move(f));
    }
}

void AsteroidsGame::removeDeadAsteroids() {
    for (const auto& a : m_asteroids)
        if (a->wasShot()) m_score += scoreForSize(a->size);

    m_asteroids.erase(
        std::remove_if(m_asteroids.begin(), m_asteroids.end(),
                       [](const auto& a) { return a->wasShot(); }),
        m_asteroids.end());
}

void AsteroidsGame::advanceWaveIfCleared(float dt) {
    if (!m_asteroids.empty()) return;

    if (m_waveTimer < 0.f)
        m_waveTimer = WAVE_DELAY;

    m_waveTimer -= dt;

    if (m_waveTimer <= 0.f) {
        m_waveTimer = -1.f;
        spawnWave(++m_wave);
    }
}

void AsteroidsGame::tryFireBullet() {
    if (auto shot = m_ship->tryShoot()) {
        m_bullets.push_back(std::make_unique<Bullet>(
            shot->pos, shot->direction * Bullet::SPEED,
            m_sheet->getFrameUVs(128), &m_sheet->texture()));
    }
}

void AsteroidsGame::removeDeadBullets() {
    m_bullets.erase(
        std::remove_if(m_bullets.begin(), m_bullets.end(),
                       [](const auto& b) { return !b->isAlive(); }),
        m_bullets.end());
}

// ---- onRender helpers -------------------------------------------------------

void AsteroidsGame::renderLivesHUD() {
    static constexpr float ICON_SIZE = 20.f;
    static constexpr float ICON_PAD  = 6.f;
    const Engine::UVRect iconUV = m_sheet->getFrameUVs(m_ship->frameIndex(), m_ship->frameCells(), m_ship->frameCells());
    for (int i = 0; i < m_lives; ++i) {
        const float x = ICON_PAD + i * (ICON_SIZE + ICON_PAD);
        m_renderer.drawTexturedRect(x, ICON_PAD, ICON_SIZE, ICON_SIZE,
                                    m_sheet->texture(),
                                    iconUV.u0, iconUV.v0, iconUV.u1, iconUV.v1);
    }
}

void AsteroidsGame::renderScore() {
    static constexpr float      s     = 3.f;
    static constexpr float      PAD   = 8.f;
    static constexpr glm::vec4  COLOR = { 1.f, 1.f, 1.f, 1.f };
    const std::string text = std::to_string(m_score);
    const float x = W - PAD - Engine::PixelFont::stringWidth(text, s);
    Engine::PixelFont::drawString(m_renderer, text, x, PAD, s, COLOR);
}

void AsteroidsGame::renderGameOver() {
    static constexpr float     TITLE_SCALE = 6.f;
    static constexpr float     SCORE_SCALE = 4.f;
    static constexpr float     HINT_SCALE  = 2.f;
    static constexpr float     GAP         = 20.f;
    static constexpr glm::vec4 COLOR       = { 1.f, 1.f, 1.f, 1.f };

    const float titleH = 7.f * TITLE_SCALE;
    const float scoreH = 7.f * SCORE_SCALE;
    const float hintH  = 7.f * HINT_SCALE;
    const float totalH = titleH + GAP + scoreH + GAP + hintH;
    const float topY   = H * 0.5f - totalH * 0.5f;

    Engine::PixelFont::drawStringCentered(m_renderer, "GAME OVER",         W * 0.5f, topY,                              TITLE_SCALE, COLOR);
    Engine::PixelFont::drawStringCentered(m_renderer, m_score,             W * 0.5f, topY + titleH + GAP,               SCORE_SCALE, COLOR);
    Engine::PixelFont::drawStringCentered(m_renderer, "PRESS R TO RESTART",W * 0.5f, topY + titleH + GAP + scoreH + GAP, HINT_SCALE,  COLOR);
}

void AsteroidsGame::renderWaveAnnouncement() {
    if (m_waveTimer < 0.f) return;

    static constexpr float LABEL_SCALE  = 7.f;
    static constexpr float NUMBER_SCALE = 10.f;
    static constexpr float GAP          = 15.f;
    static constexpr glm::vec4 COLOR    = { 1.f, 1.f, 1.f, 1.f };

    const float labelH  = 7.f * LABEL_SCALE;   // PixelFont glyphs are 7 rows tall
    const float numberH = 5.f * NUMBER_SCALE;  // SegmentFont glyphs are 5 rows tall
    const float totalH  = labelH + GAP + numberH;
    const float topY    = H * 0.5f - totalH * 0.5f;

    Engine::PixelFont::drawStringCentered  (m_renderer, "WAVE",     W * 0.5f, topY,                LABEL_SCALE,  COLOR);
    Engine::SegmentFont::drawStringCentered(m_renderer, m_wave + 1, W * 0.5f, topY + labelH + GAP, NUMBER_SCALE, COLOR);
}

void AsteroidsGame::restartGame() {
    m_asteroids.clear();
    m_bullets.clear();
    m_lives     = STARTING_LIVES;
    m_score     = 0;
    m_wave      = 1;
    m_waveTimer = -1.f;
    m_gameOver  = false;
    m_ship->reset();
    spawnInitialAsteroidRing();
}

int AsteroidsGame::scoreForSize(AsteroidSize size) const {
    switch (size) {
        case AsteroidSize::Large:  return SCORE_LARGE;
        case AsteroidSize::Medium: return SCORE_MEDIUM;
        case AsteroidSize::Small:  return SCORE_SMALL;
        default: return 0;
    }
}

// ---- spawning ---------------------------------------------------------------

void AsteroidsGame::spawnInitialAsteroidRing() {
    const glm::vec2 playerStart(W * 0.5f, H * 0.5f);

    for (int i = 0; i < STARTING_ASTEROID_COUNT; ++i) {
        const float baseAngle   = (glm::two_pi<float>() / STARTING_ASTEROID_COUNT) * i;
        const float jitter      = std::uniform_real_distribution<float>(
                                      -glm::pi<float>() / 6.f,
                                       glm::pi<float>() / 6.f)(m_rng);
        const float spawnRadius = STARTING_ASTEROID_MIN_DIST_FROM_PLAYER
                                + std::uniform_real_distribution<float>(0.f, 120.f)(m_rng);

        glm::vec2 pos = playerStart + glm::vec2(std::cos(baseAngle + jitter),
                                                std::sin(baseAngle + jitter)) * spawnRadius;
        pos.x = std::clamp(pos.x, 32.f, static_cast<float>(W) - 32.f);
        pos.y = std::clamp(pos.y, 32.f, static_cast<float>(H) - 32.f);

        m_asteroids.push_back(Asteroid::spawnLarge(pos, m_rng));
    }
}

void AsteroidsGame::spawnWave(int wave) {
    const int count = std::min(STARTING_ASTEROID_COUNT + (wave - 1) * 2, MAX_ASTEROIDS_PER_WAVE);
    const glm::vec2 shipPos = m_ship->pos();

    for (int i = 0; i < count; ++i) {
        glm::vec2 pos;
        do { pos = randomEdgePosition(); }
        while (glm::distance(pos, shipPos) < WAVE_SPAWN_MIN_DIST_FROM_SHIP);

        m_asteroids.push_back(Asteroid::spawnLarge(pos, m_rng));
    }
}

glm::vec2 AsteroidsGame::randomEdgePosition() {
    const int   side = std::uniform_int_distribution<int>(0, 3)(m_rng);
    const float u    = std::uniform_real_distribution<float>(0.f, 1.f)(m_rng);
    switch (side) {
        case 0:  return { u * W,      -16.f    };  // top
        case 1:  return { u * W,       H + 16.f };  // bottom
        case 2:  return { -16.f,       u * H    };  // left
        default: return {  W + 16.f,   u * H    };  // right
    }
}
