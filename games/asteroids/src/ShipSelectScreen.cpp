#include "ShipSelectScreen.h"
#include "GameConstants.h"
#include "ShipConfig.h"
#include <engine/core/Input.h>
#include <engine/renderer/PixelFont.h>
#include <GLFW/glfw3.h>

ShipSelectScreen::ShipSelectScreen(GameContext& ctx) : m_ctx(ctx) {}

void ShipSelectScreen::preStep(float dt) {
    for (auto& a : m_ctx.bgAsteroids)
        a->update(dt, W, H);
}

Screen ShipSelectScreen::update(float dt) {
    (void)dt;
    if (Engine::Input::isKeyPressed(GLFW_KEY_Q))      return Screen::Quit;
    if (Engine::Input::isKeyPressed(GLFW_KEY_ESCAPE)) return Screen::Title;

    const int shipCount = static_cast<int>(ShipConfigs::All.size());
    if (Engine::Input::isKeyPressed(GLFW_KEY_LEFT) || Engine::Input::isKeyPressed(GLFW_KEY_A))
        m_ctx.selectedShip = (m_ctx.selectedShip - 1 + shipCount) % shipCount;
    if (Engine::Input::isKeyPressed(GLFW_KEY_RIGHT) || Engine::Input::isKeyPressed(GLFW_KEY_D))
        m_ctx.selectedShip = (m_ctx.selectedShip + 1) % shipCount;

    if (Engine::Input::isKeyPressed(GLFW_KEY_ENTER) || Engine::Input::isKeyPressed(GLFW_KEY_KP_ENTER))
        return Screen::Playing;

    return Screen::ShipSelect;
}

void ShipSelectScreen::render() {
    for (const auto& a : m_ctx.bgAsteroids)
        a->render(m_ctx.renderer, *m_ctx.sheet);

    static constexpr float     HEADER_SCALE = 4.f;
    static constexpr float     PREVIEW_SIZE = 96.f;
    static constexpr float     GAP          = 32.f;
    static constexpr float     HINT_SCALE   = 2.f;
    static constexpr glm::vec4 WHITE        = { 1.f, 1.f, 1.f, 1.f };
    static constexpr glm::vec4 GOLD         = { 1.f, 0.85f, 0.1f, 1.f };

    Engine::PixelFont::drawStringCentered(m_ctx.renderer, "SELECT SHIP", W * 0.5f, H * 0.12f, HEADER_SCALE, WHITE);

    const int   shipCount = static_cast<int>(ShipConfigs::All.size());
    const float totalW  = shipCount * PREVIEW_SIZE + (shipCount - 1) * GAP;
    const float startX  = W * 0.5f - totalW * 0.5f;
    const float previewY = H * 0.35f;
    const float nameY   = previewY + PREVIEW_SIZE + 16.f;
    const Engine::Texture& tex = m_ctx.sheet->texture();

    for (int i = 0; i < shipCount; ++i) {
        const bool       selected = (i == m_ctx.selectedShip);
        const float      x        = startX + i * (PREVIEW_SIZE + GAP);
        const float      cx       = x + PREVIEW_SIZE * 0.5f;
        const Engine::UVRect uv   = m_ctx.sheet->getFrameUVs(ShipConfigs::All[i].shipFrame, 2, 2);
        const glm::vec4  tint     = selected ? glm::vec4{1.f, 1.f, 1.f, 1.f} : glm::vec4{0.35f, 0.35f, 0.35f, 1.f};

        m_ctx.renderer.drawTexturedRect(x, previewY, PREVIEW_SIZE, PREVIEW_SIZE,
                                        tex, uv.u0, uv.v0, uv.u1, uv.v1, 0.f, tint);

        const glm::vec4& nameColor = selected ? GOLD : WHITE;
        Engine::PixelFont::drawStringCentered(m_ctx.renderer, ShipConfigs::All[i].name, cx, nameY, 2.f, nameColor);

        if (selected)
            Engine::PixelFont::drawStringCentered(m_ctx.renderer, "^", cx, previewY - 16.f, 2.f, GOLD);
    }

    Engine::PixelFont::drawStringCentered(m_ctx.renderer, "< >  ENTER TO START",
                                          W * 0.5f, H * 0.72f, HINT_SCALE, WHITE);
}
