#include "TitleScreen.h"
#include "AsteroidsConfig.h"
#include "Asteroid.h"
#include <engine/core/Input.h>
#include <engine/renderer/PixelFont.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

TitleScreen::TitleScreen(GameContext& ctx) : m_ctx(ctx) {}

void TitleScreen::onEnter() {
    static constexpr int COUNT = 8;
    for (int i = 0; i < COUNT; ++i) {
        glm::vec2 pos = {
            std::uniform_real_distribution<float>(0.f, static_cast<float>(W))(m_ctx.rng),
            std::uniform_real_distribution<float>(0.f, static_cast<float>(H))(m_ctx.rng)
        };
        m_ctx.bgAsteroids.push_back(Asteroid::spawnLarge(pos, m_ctx.rng));
    }
}

void TitleScreen::preStep(float dt) {
    for (auto& a : m_ctx.bgAsteroids)
        a->update(dt, W, H);
}

Screen TitleScreen::update(float dt) {
    (void)dt;
    if (Engine::Input::isKeyPressed(GLFW_KEY_Q)) return Screen::Quit;

    m_ctx.titleMenu.update();
    if (m_ctx.titleMenu.confirmed()) {
        if (m_ctx.titleMenu.selectedIndex() == 0) return Screen::ShipSelect;
        return Screen::Quit;
    }
    return Screen::Title;
}

void TitleScreen::render() {
    for (const auto& a : m_ctx.bgAsteroids)
        a->render(m_ctx.renderer, *m_ctx.sheet);

    static constexpr float     TITLE_SCALE = 8.f;
    static constexpr glm::vec4 WHITE       = { 1.f, 1.f, 1.f, 1.f };
    static constexpr glm::vec4 GOLD        = { 1.f, 0.85f, 0.1f, 1.f };

    const float titleH = 7.f * TITLE_SCALE;
    const float titleY = H * 0.28f;
    Engine::PixelFont::drawStringCentered(m_ctx.renderer, "ASTEROIDS", W * 0.5f, titleY, TITLE_SCALE, WHITE);

    if (m_ctx.highScore > 0) {
        static constexpr float HS_SCALE = 2.f;
        const float hsY = titleY + titleH + 12.f;
        Engine::PixelFont::drawStringCentered(m_ctx.renderer, "BEST",         W * 0.5f - 40.f, hsY, HS_SCALE, GOLD);
        Engine::PixelFont::drawStringCentered(m_ctx.renderer, m_ctx.highScore, W * 0.5f + 40.f, hsY, HS_SCALE, GOLD);
    }

    const float menuW = Engine::PixelFont::stringWidth("  EXIT", 3.f);
    const float menuX = W * 0.5f - menuW * 0.5f;
    m_ctx.titleMenu.draw(m_ctx.renderer, menuX, H * 0.58f);
}
