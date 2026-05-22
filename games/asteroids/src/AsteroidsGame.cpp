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
    , m_titleMenu({"PLAY", "EXIT"}, 3.f)
{
    auto texture = std::make_shared<Engine::Texture>("games/asteroids/assets/asteroids-arcade.png");
    m_sheet      = std::make_shared<Engine::SpriteSheet>(texture, 16, 16);
    m_ship.emplace(m_sheet);

    m_saveData  = Engine::SaveData::load("asteroids");
    m_highScore = m_saveData.getInt("high_score", 0);

    spawnBgAsteroids();
}

// ---- core loop --------------------------------------------------------------

void AsteroidsGame::preStep(float dt) {
    for (auto& a : m_bgAsteroids)
        a->update(dt, W, H);

    if (m_screen != Screen::Playing) return;

    m_ship->update(dt, W, H);

    for (auto& b : m_bullets)
        b->update(dt);

    for (auto& a : m_asteroids)
        a->update(dt, W, H);
}

void AsteroidsGame::onUpdate(float dt) {
    if (Engine::Input::isKeyPressed(GLFW_KEY_Q)) quit();

    if (m_screen == Screen::Title) {
        updateTitleScreen(dt);
        return;
    }

    if (m_screen == Screen::GameOver) {
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

    if (m_screen == Screen::Title) {
        renderTitleScreen();
        return;
    }

    for (const auto& a : m_asteroids)
        a->render(m_renderer, *m_sheet);

    m_ship->render(m_renderer);

    for (const auto& b : m_bullets)
        b->render(m_renderer);

    renderScore();
    renderLivesHUD();
    renderWaveAnnouncement();
    if (m_screen == Screen::GameOver) renderGameOver();
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
        m_screen = Screen::GameOver;
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
    for (const auto& a : m_asteroids) {
        if (!a->wasShot()) continue;
        m_score += scoreForSize(a->size);
        if (m_score > m_highScore) {
            m_highScore    = m_score;
            m_newHighScore = true;
            m_saveData.setInt("high_score", m_highScore);
            m_saveData.save();
        }
    }

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
    static constexpr float     s     = 3.f;
    static constexpr float     PAD   = 8.f;
    static constexpr glm::vec4 WHITE = { 1.f, 1.f, 1.f, 1.f };
    static constexpr glm::vec4 GOLD  = { 1.f, 0.85f, 0.1f, 1.f };

    const std::string scoreText = std::to_string(m_score);
    const float scoreX = W - PAD - Engine::PixelFont::stringWidth(scoreText, s);
    Engine::PixelFont::drawString(m_renderer, scoreText, scoreX, PAD, s, WHITE);

    const std::string hiText = std::to_string(m_highScore);
    const float hiX = W * 0.5f - Engine::PixelFont::stringWidth(hiText, s) * 0.5f;
    Engine::PixelFont::drawString(m_renderer, hiText, hiX, PAD, s, GOLD);
}

void AsteroidsGame::renderGameOver() {
    static constexpr float     TITLE_SCALE = 6.f;
    static constexpr float     SCORE_SCALE = 4.f;
    static constexpr float     NEW_HS_SCALE = 3.f;
    static constexpr float     HINT_SCALE  = 2.f;
    static constexpr float     GAP         = 20.f;
    static constexpr glm::vec4 WHITE       = { 1.f, 1.f, 1.f, 1.f };
    static constexpr glm::vec4 GOLD        = { 1.f, 0.85f, 0.1f, 1.f };

    const float titleH  = 7.f * TITLE_SCALE;
    const float scoreH  = 7.f * SCORE_SCALE;
    const float newHsH  = m_newHighScore ? 7.f * NEW_HS_SCALE + GAP : 0.f;
    const float hintH   = 7.f * HINT_SCALE;
    const float totalH  = titleH + GAP + scoreH + newHsH + GAP + hintH;
    const float topY    = H * 0.5f - totalH * 0.5f;

    Engine::PixelFont::drawStringCentered(m_renderer, "GAME OVER",          W * 0.5f, topY,                    TITLE_SCALE, WHITE);
    Engine::PixelFont::drawStringCentered(m_renderer, m_score,              W * 0.5f, topY + titleH + GAP,     SCORE_SCALE, WHITE);

    if (m_newHighScore)
        Engine::PixelFont::drawStringCentered(m_renderer, "NEW HIGH SCORE", W * 0.5f, topY + titleH + GAP + scoreH + GAP, NEW_HS_SCALE, GOLD);

    Engine::PixelFont::drawStringCentered(m_renderer, "PRESS R TO RESTART", W * 0.5f, topY + titleH + GAP + scoreH + newHsH + GAP, HINT_SCALE, WHITE);
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
    m_lives        = STARTING_LIVES;
    m_score        = 0;
    m_wave         = 1;
    m_waveTimer    = -1.f;
    m_screen       = Screen::Playing;
    m_newHighScore = false;
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

// ---- title screen -----------------------------------------------------------

void AsteroidsGame::spawnBgAsteroids() {
    static constexpr int BG_ASTEROID_COUNT = 8;
    for (int i = 0; i < BG_ASTEROID_COUNT; ++i) {
        glm::vec2 pos = {
            std::uniform_real_distribution<float>(0.f, static_cast<float>(W))(m_rng),
            std::uniform_real_distribution<float>(0.f, static_cast<float>(H))(m_rng)
        };
        m_bgAsteroids.push_back(Asteroid::spawnLarge(pos, m_rng));
    }
}

void AsteroidsGame::updateTitleScreen(float dt) {
    (void)dt;
    m_titleMenu.update();
    if (m_titleMenu.confirmed()) {
        if (m_titleMenu.selectedIndex() == 0) {
            m_screen = Screen::Playing;
            spawnInitialAsteroidRing();
        } else {
            quit();
        }
    }
}

void AsteroidsGame::renderTitleScreen() {
    for (const auto& a : m_bgAsteroids)
        a->render(m_renderer, *m_sheet);

    static constexpr float     TITLE_SCALE = 8.f;
    static constexpr glm::vec4 WHITE       = { 1.f, 1.f, 1.f, 1.f };
    static constexpr glm::vec4 GOLD        = { 1.f, 0.85f, 0.1f, 1.f };

    const float titleH = 7.f * TITLE_SCALE;
    const float titleY = H * 0.28f;
    Engine::PixelFont::drawStringCentered(m_renderer, "ASTEROIDS", W * 0.5f, titleY, TITLE_SCALE, WHITE);

    if (m_highScore > 0) {
        static constexpr float HS_SCALE = 2.f;
        const float hsY = titleY + titleH + 12.f;
        Engine::PixelFont::drawStringCentered(m_renderer, "BEST", W * 0.5f - 40.f, hsY, HS_SCALE, GOLD);
        Engine::PixelFont::drawStringCentered(m_renderer, m_highScore,  W * 0.5f + 40.f, hsY, HS_SCALE, GOLD);
    }

    const float menuW  = Engine::PixelFont::stringWidth("  EXIT", 3.f);
    const float menuX  = W * 0.5f - menuW * 0.5f;
    const float menuY  = H * 0.58f;
    m_titleMenu.draw(m_renderer, menuX, menuY);
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
