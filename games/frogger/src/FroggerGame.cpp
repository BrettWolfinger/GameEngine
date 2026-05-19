#include "FroggerGame.h"
#include "FroggerConfig.h"
#include <engine/renderer/Texture.h>
#include <engine/core/Input.h>
#include <GLFW/glfw3.h>

FroggerGame::FroggerGame()
    : Engine::Application("Frogger", W, H)
{
    auto texture = std::make_shared<Engine::Texture>("games/frogger/assets/frogger_sprite_sheet.png");
    m_sheet      = std::make_shared<Engine::SpriteSheet>(texture, SHEET_COLS, SHEET_ROWS);
    m_frog.emplace(m_sheet);
}

void FroggerGame::onUpdate(float dt) {
    if (Engine::Input::isKeyPressed(GLFW_KEY_Q))
        quit();

    m_frog->update(dt);
}

void FroggerGame::renderBackground() {
    const glm::vec4 grass { 0.10f, 0.35f, 0.10f, 1.f };
    const glm::vec4 river { 0.05f, 0.15f, 0.45f, 1.f };
    const glm::vec4 road  { 0.18f, 0.18f, 0.18f, 1.f };
    const glm::vec4 goal  { 0.03f, 0.10f, 0.03f, 1.f };

    for (int row = 0; row < ROWS; ++row) {
        const float ry = static_cast<float>(row * TILE);
        glm::vec4 color;
        if      (row >= RIVER_FIRST_ROW && row <= RIVER_LAST_ROW) color = river;
        else if (row >= ROAD_FIRST_ROW  && row <= ROAD_LAST_ROW)  color = road;
        else                                                        color = grass;
        m_renderer.drawRect(0.f, ry, static_cast<float>(W), static_cast<float>(TILE), color);
    }

    for (int i = 0; i < HOME_SLOT_COUNT; ++i)
        m_renderer.drawRect(static_cast<float>(HOME_SLOTS[i] * TILE), 0.f,
                            static_cast<float>(TILE), static_cast<float>(TILE), goal);
}

void FroggerGame::onRender() {
    m_renderer.beginScene(W, H);
    renderBackground();
    m_frog->render(m_renderer);
}
