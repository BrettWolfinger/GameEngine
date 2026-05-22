#include "GameOverScreen.h"
#include "AsteroidsConfig.h"
#include <engine/core/Input.h>
#include <engine/renderer/PixelFont.h>
#include <GLFW/glfw3.h>

GameOverScreen::GameOverScreen(GameContext& ctx) : m_ctx(ctx) {}

Screen GameOverScreen::update(float dt) {
    (void)dt;
    if (Engine::Input::isKeyPressed(GLFW_KEY_Q)) return Screen::Quit;
    if (Engine::Input::isKeyPressed(GLFW_KEY_R)) return Screen::ShipSelect;
    return Screen::GameOver;
}

void GameOverScreen::render() {
    static constexpr float     TITLE_SCALE  = 6.f;
    static constexpr float     SCORE_SCALE  = 4.f;
    static constexpr float     NEW_HS_SCALE = 3.f;
    static constexpr float     HINT_SCALE   = 2.f;
    static constexpr float     GAP          = 20.f;
    static constexpr glm::vec4 WHITE        = { 1.f, 1.f, 1.f, 1.f };
    static constexpr glm::vec4 GOLD         = { 1.f, 0.85f, 0.1f, 1.f };

    const float titleH = 7.f * TITLE_SCALE;
    const float scoreH = 7.f * SCORE_SCALE;
    const float newHsH = m_ctx.newHighScore ? 7.f * NEW_HS_SCALE + GAP : 0.f;
    const float hintH  = 7.f * HINT_SCALE;
    const float totalH = titleH + GAP + scoreH + newHsH + GAP + hintH;
    const float topY   = H * 0.5f - totalH * 0.5f;

    Engine::PixelFont::drawStringCentered(m_ctx.renderer, "GAME OVER",          W * 0.5f, topY,                                   TITLE_SCALE, WHITE);
    Engine::PixelFont::drawStringCentered(m_ctx.renderer, m_ctx.score,          W * 0.5f, topY + titleH + GAP,                    SCORE_SCALE, WHITE);

    if (m_ctx.newHighScore)
        Engine::PixelFont::drawStringCentered(m_ctx.renderer, "NEW HIGH SCORE", W * 0.5f, topY + titleH + GAP + scoreH + GAP,    NEW_HS_SCALE, GOLD);

    Engine::PixelFont::drawStringCentered(m_ctx.renderer, "PRESS R TO RESTART", W * 0.5f, topY + titleH + GAP + scoreH + newHsH + GAP, HINT_SCALE, WHITE);
}
