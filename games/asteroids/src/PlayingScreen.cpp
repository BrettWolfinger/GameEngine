#include "PlayingScreen.h"
#include "AsteroidsConfig.h"
#include "ShipConfig.h"
#include <engine/core/Input.h>
#include <engine/renderer/PixelFont.h>
#include <engine/renderer/SegmentFont.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/constants.hpp>
#include <algorithm>
#include <cmath>

PlayingScreen::PlayingScreen(GameContext& ctx) : m_ctx(ctx) {}

void PlayingScreen::onEnter() {
    m_ctx.bgAsteroids.clear();
    m_ctx.ship.emplace(m_ctx.sheet, ShipConfigs::All[m_ctx.selectedShip]);
    spawnInitialAsteroidRing();
}

void PlayingScreen::spawnInitialAsteroidRing() {
    const glm::vec2 playerStart(W * 0.5f, H * 0.5f);

    for (int i = 0; i < STARTING_ASTEROID_COUNT; ++i) {
        const float baseAngle   = (glm::two_pi<float>() / STARTING_ASTEROID_COUNT) * i;
        const float jitter      = std::uniform_real_distribution<float>(
                                      -glm::pi<float>() / 6.f,
                                       glm::pi<float>() / 6.f)(m_ctx.rng);
        const float spawnRadius = STARTING_ASTEROID_MIN_DIST_FROM_PLAYER
                                + std::uniform_real_distribution<float>(0.f, 120.f)(m_ctx.rng);

        glm::vec2 pos = playerStart + glm::vec2(std::cos(baseAngle + jitter),
                                                std::sin(baseAngle + jitter)) * spawnRadius;
        pos.x = std::clamp(pos.x, 32.f, static_cast<float>(W) - 32.f);
        pos.y = std::clamp(pos.y, 32.f, static_cast<float>(H) - 32.f);

        m_ctx.asteroids.push_back(Asteroid::spawnLarge(pos, m_ctx.rng));
    }
}

// ---- core loop --------------------------------------------------------------

void PlayingScreen::preStep(float dt) {
    if (m_ctx.ship) m_ctx.ship->update(dt, W, H);
    for (auto& b : m_ctx.bullets)  b->update(dt);
    for (auto& a : m_ctx.asteroids) a->update(dt, W, H);
}

Screen PlayingScreen::update(float dt) {
    if (Engine::Input::isKeyPressed(GLFW_KEY_Q)) return Screen::Quit;

#ifdef ENABLE_DEV_KEYS
    handleDevInput();
#endif

    if (m_ctx.ship && m_ctx.ship->wasHit()) handleShipHit();

    spawnAsteroidFragments();
    removeDeadAsteroids();
    advanceWaveIfCleared(dt);
    tryFireBullet();
    removeDeadBullets();

    if (m_ctx.lives <= 0) return Screen::GameOver;
    return Screen::Playing;
}

void PlayingScreen::render() {
    for (const auto& a : m_ctx.asteroids) a->render(m_ctx.renderer, *m_ctx.sheet);
    if (m_ctx.ship) m_ctx.ship->render(m_ctx.renderer);
    for (const auto& b : m_ctx.bullets)  b->render(m_ctx.renderer);

    renderScore();
    renderLivesHUD();
    renderWaveAnnouncement();
}

// ---- update helpers ---------------------------------------------------------

#ifdef ENABLE_DEV_KEYS
void PlayingScreen::handleDevInput() {
    if (Engine::Input::isKeyPressed(GLFW_KEY_C))
        m_ctx.asteroids.clear();
}
#endif

void PlayingScreen::handleShipHit() {
    m_ctx.bullets.clear();
    if (--m_ctx.lives > 0)
        m_ctx.ship->reset();
}

void PlayingScreen::spawnAsteroidFragments() {
    const size_t n = m_ctx.asteroids.size();
    for (size_t i = 0; i < n; ++i) {
        if (!m_ctx.asteroids[i]->wasShot()) continue;
        for (auto& f : m_ctx.asteroids[i]->split())
            m_ctx.asteroids.push_back(std::move(f));
    }
}

void PlayingScreen::removeDeadAsteroids() {
    for (const auto& a : m_ctx.asteroids) {
        if (!a->wasShot()) continue;
        m_ctx.score += scoreForSize(a->size);
        if (m_ctx.score >= m_ctx.nextLifeScore) {
            ++m_ctx.lives;
            m_ctx.nextLifeScore += 1000;
        }
        if (m_ctx.score > m_ctx.highScore) {
            m_ctx.highScore    = m_ctx.score;
            m_ctx.newHighScore = true;
            m_ctx.saveData.setInt("high_score", m_ctx.highScore);
            m_ctx.saveData.save();
        }
    }

    m_ctx.asteroids.erase(
        std::remove_if(m_ctx.asteroids.begin(), m_ctx.asteroids.end(),
                       [](const auto& a) { return a->wasShot(); }),
        m_ctx.asteroids.end());
}

void PlayingScreen::advanceWaveIfCleared(float dt) {
    if (!m_ctx.asteroids.empty()) return;

    if (m_ctx.waveTimer < 0.f)
        m_ctx.waveTimer = WAVE_DELAY;

    m_ctx.waveTimer -= dt;

    if (m_ctx.waveTimer <= 0.f) {
        m_ctx.waveTimer = -1.f;
        spawnWave(++m_ctx.wave);
    }
}

void PlayingScreen::tryFireBullet() {
    if (!m_ctx.ship) return;
    if (auto shot = m_ctx.ship->tryShoot()) {
        m_ctx.bullets.push_back(std::make_unique<Bullet>(
            shot->pos, shot->direction * Bullet::SPEED,
            m_ctx.sheet->getFrameUVs(128), &m_ctx.sheet->texture()));
    }
}

void PlayingScreen::removeDeadBullets() {
    m_ctx.bullets.erase(
        std::remove_if(m_ctx.bullets.begin(), m_ctx.bullets.end(),
                       [](const auto& b) { return !b->isAlive(); }),
        m_ctx.bullets.end());
}

// ---- render helpers ---------------------------------------------------------

void PlayingScreen::renderScore() {
    static constexpr float     s     = 3.f;
    static constexpr float     PAD   = 8.f;
    static constexpr glm::vec4 WHITE = { 1.f, 1.f, 1.f, 1.f };
    static constexpr glm::vec4 GOLD  = { 1.f, 0.85f, 0.1f, 1.f };

    const std::string scoreText = std::to_string(m_ctx.score);
    const float scoreX = W - PAD - Engine::PixelFont::stringWidth(scoreText, s);
    Engine::PixelFont::drawString(m_ctx.renderer, scoreText, scoreX, PAD, s, WHITE);

    const std::string hiText = std::to_string(m_ctx.highScore);
    const float hiX = W * 0.5f - Engine::PixelFont::stringWidth(hiText, s) * 0.5f;
    Engine::PixelFont::drawString(m_ctx.renderer, hiText, hiX, PAD, s, GOLD);
}

void PlayingScreen::renderLivesHUD() {
    if (!m_ctx.ship) return;
    static constexpr float ICON_SIZE = 20.f;
    static constexpr float ICON_PAD  = 6.f;
    const Engine::UVRect iconUV = m_ctx.sheet->getFrameUVs(m_ctx.ship->frameIndex(),
                                                            m_ctx.ship->frameCells(),
                                                            m_ctx.ship->frameCells());
    for (int i = 0; i < m_ctx.lives; ++i) {
        const float x = ICON_PAD + i * (ICON_SIZE + ICON_PAD);
        m_ctx.renderer.drawTexturedRect(x, ICON_PAD, ICON_SIZE, ICON_SIZE,
                                        m_ctx.sheet->texture(),
                                        iconUV.u0, iconUV.v0, iconUV.u1, iconUV.v1);
    }
}

void PlayingScreen::renderWaveAnnouncement() {
    if (m_ctx.waveTimer < 0.f) return;

    static constexpr float LABEL_SCALE  = 7.f;
    static constexpr float NUMBER_SCALE = 10.f;
    static constexpr float GAP          = 15.f;
    static constexpr glm::vec4 COLOR    = { 1.f, 1.f, 1.f, 1.f };

    const float labelH = 7.f * LABEL_SCALE;
    const float topY   = H * 0.5f - (labelH + GAP + 5.f * NUMBER_SCALE) * 0.5f;

    Engine::PixelFont::drawStringCentered  (m_ctx.renderer, "WAVE",          W * 0.5f, topY,                LABEL_SCALE,  COLOR);
    Engine::SegmentFont::drawStringCentered(m_ctx.renderer, m_ctx.wave + 1,  W * 0.5f, topY + labelH + GAP, NUMBER_SCALE, COLOR);
}

// ---- spawning ---------------------------------------------------------------

void PlayingScreen::spawnWave(int wave) {
    const int count = std::min(4 + (wave - 1) * 2, MAX_ASTEROIDS_PER_WAVE);
    const glm::vec2 shipPos = m_ctx.ship ? m_ctx.ship->pos() : glm::vec2(W * 0.5f, H * 0.5f);

    for (int i = 0; i < count; ++i) {
        glm::vec2 pos;
        do { pos = randomEdgePosition(); }
        while (glm::distance(pos, shipPos) < WAVE_SPAWN_MIN_DIST_FROM_SHIP);
        m_ctx.asteroids.push_back(Asteroid::spawnLarge(pos, m_ctx.rng));
    }
}

glm::vec2 PlayingScreen::randomEdgePosition() {
    const int   side = std::uniform_int_distribution<int>(0, 3)(m_ctx.rng);
    const float u    = std::uniform_real_distribution<float>(0.f, 1.f)(m_ctx.rng);
    switch (side) {
        case 0:  return { u * W,      -16.f     };
        case 1:  return { u * W,       H + 16.f };
        case 2:  return { -16.f,       u * H    };
        default: return {  W + 16.f,   u * H    };
    }
}

int PlayingScreen::scoreForSize(AsteroidSize size) const {
    switch (size) {
        case AsteroidSize::Large:  return SCORE_LARGE;
        case AsteroidSize::Medium: return SCORE_MEDIUM;
        case AsteroidSize::Small:  return SCORE_SMALL;
        default: return 0;
    }
}
