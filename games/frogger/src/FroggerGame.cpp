#include "FroggerGame.h"
#include "FroggerConfig.h"
#include <engine/renderer/Texture.h>
#include <engine/core/Input.h>
#include <engine/renderer/PixelFont.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/constants.hpp>


FroggerGame::FroggerGame()
    : Engine::Application("Frogger", W, H)
{
    auto texture = std::make_shared<Engine::Texture>("games/frogger/assets/frogger_sprite_sheet.png");
    m_sheet      = std::make_shared<Engine::SpriteSheet>(texture, SHEET_COLS, SHEET_ROWS);
    m_frog.emplace(m_sheet);

    for (const auto& lane : LANE_CONFIGS) {
        for (int i = 0; i < lane.count; ++i) {
            float startX = static_cast<float>(i) * lane.spacing;
            if (lane.direction < 0)
                startX = W - startX;
            m_vehicles.emplace_back(startX, lane.row, lane.type, lane.speed, lane.direction);
        }
    }

    for (const auto& lane : RIVER_LANE_CONFIGS) {
        for (int i = 0; i < lane.count; ++i) {
            float startX = static_cast<float>(i) * lane.spacing;
            if (lane.direction < 0)
                startX = W - startX;
            m_platforms.emplace_back(startX, lane.row, lane.type, lane.tileWidth, lane.speed, lane.direction);
        }
    }
}

void FroggerGame::die() {
    m_deathX     = m_frog->pixelX();
    m_deathY     = static_cast<float>(m_frog->row() * TILE);
    m_deathTimer = DEATH_DISPLAY_DURATION;
    m_frog->reset();
    if (--m_lives <= 0)
        m_state = GameState::GameOver;
}

void FroggerGame::restartGame() {
    m_lives          = LIVES_START;
    m_allHomesFilled = false;
    m_state          = GameState::Playing;
    m_deathTimer     = 0.f;
    for (bool& s : m_filledSlots) s = false;
    m_frog->reset();
}

void FroggerGame::onUpdate(float dt) {
    if (Engine::Input::isKeyPressed(GLFW_KEY_Q))
        quit();

    if (m_state != GameState::Playing) {
        if (Engine::Input::isKeyPressed(GLFW_KEY_SPACE))
            restartGame();
        return;
    }

    m_frog->update(dt);

#ifdef ENABLE_DEV_KEYS
    if (Engine::Input::isKeyPressed(GLFW_KEY_F1))
        m_frog->teleport(6, MEDIAN_ROW);
    if (Engine::Input::isKeyPressed(GLFW_KEY_F2)) {
        for (bool& s : m_filledSlots) s = true;
        m_allHomesFilled = true;
        m_state = GameState::Win;
    }
#endif

    if (m_deathTimer > 0.f)
        m_deathTimer -= dt;

    for (auto& v : m_vehicles)
        v.update(dt);
    for (auto& p : m_platforms)
        p.update(dt);

    const int   frogRow = m_frog->row();
    const float frogPx  = m_frog->pixelX();

    // Home row: valid unfilled slot = success; anything else = death
    if (frogRow == HOME_ROW) {
        int slotIdx = -1;
        for (int i = 0; i < HOME_SLOT_COUNT; ++i)
            if (m_frog->col() == HOME_SLOTS[i]) { slotIdx = i; break; }

        if (slotIdx >= 0 && !m_filledSlots[slotIdx]) {
            m_filledSlots[slotIdx] = true;
            m_frog->reset();
            m_allHomesFilled = true;
            for (int i = 0; i < HOME_SLOT_COUNT; ++i)
                if (!m_filledSlots[i]) { m_allHomesFilled = false; break; }
            if (m_allHomesFilled)
                m_state = GameState::Win;
        } else {
            die();
        }
        return;
    }

    // River zone: must be on a platform or drown
    if (frogRow >= RIVER_FIRST_ROW && frogRow <= RIVER_LAST_ROW) {
        const Platform* riding = nullptr;
        for (const auto& p : m_platforms) {
            if (p.row() != frogRow) continue;
            const float pw = static_cast<float>(p.tileWidth() * TILE);
            if (frogPx < p.x() + pw && frogPx + TILE > p.x()) {
                riding = &p;
                break;
            }
        }
        if (riding) {
            m_frog->applyRide(riding->velocityX() * dt);
            if (m_frog->col() < 0 || m_frog->col() >= COLS)
                die();
        } else {
            die();
        }
    }

    // Road zone: vehicle collision = death
    if (frogRow >= ROAD_FIRST_ROW && frogRow <= ROAD_LAST_ROW) {
        for (const auto& v : m_vehicles) {
            if (v.row() != frogRow) continue;
            const float vw = static_cast<float>(v.tileWidth() * TILE);
            if (frogPx < v.x() + vw && frogPx + TILE > v.x()) {
                die();
                break;
            }
        }
    }
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

    const Engine::UVRect uvs = m_sheet->getFrameUVs(HOME_FILLED_FRAME);
    for (int i = 0; i < HOME_SLOT_COUNT; ++i) {
        if (!m_filledSlots[i]) continue;
        m_renderer.drawTexturedRect(static_cast<float>(HOME_SLOTS[i] * TILE), 0.f,
                                    static_cast<float>(TILE), static_cast<float>(TILE),
                                    m_sheet->texture(),
                                    uvs.u0, uvs.v0, uvs.u1, uvs.v1);
    }
}

void FroggerGame::renderHUD() {
    // Life icons: frog sprites in bottom-left of the safe zone
    const Engine::UVRect uvs = m_sheet->getFrameUVs(0);
    const float iconSize = static_cast<float>(TILE) * 0.6f;
    const float iconY    = static_cast<float>((ROWS - 1) * TILE) + (TILE - iconSize) * 0.5f;
    for (int i = 0; i < m_lives; ++i) {
        const float iconX = static_cast<float>(i) * (iconSize + 4.f) + 4.f;
        m_renderer.drawTexturedRect(iconX, iconY, iconSize, iconSize,
                                    m_sheet->texture(),
                                    uvs.u0, uvs.v0, uvs.u1, uvs.v1);
    }
}

void FroggerGame::renderEndScreen(std::string_view title, const glm::vec4& titleColor) {
    // Dim overlay
    m_renderer.drawRect(0.f, 0.f, static_cast<float>(W), static_cast<float>(H),
                        { 0.f, 0.f, 0.f, 0.55f });

    const float cx = W * 0.5f;
    const float scale = 3.f;
    Engine::PixelFont::drawStringCentered(m_renderer, title,                    cx, H * 0.38f, scale,   titleColor);
    Engine::PixelFont::drawStringCentered(m_renderer, "PRESS SPACE TO PLAY AGAIN", cx, H * 0.52f, 1.5f, { 1.f, 1.f, 1.f, 1.f });
}

void FroggerGame::onRender() {
    m_renderer.beginScene(W, H);
    renderBackground();

    for (auto& p : m_platforms)
        p.render(m_renderer, *m_sheet);
    for (auto& v : m_vehicles)
        v.render(m_renderer, *m_sheet);

    m_frog->render(m_renderer);

    if (m_deathTimer > 0.f) {
        const Engine::UVRect skullUVs = m_sheet->getFrameUVs(SKULL_FRAME);
        m_renderer.drawTexturedRect(m_deathX, m_deathY, TILE, TILE,
                                    m_sheet->texture(),
                                    skullUVs.u0, skullUVs.v0, skullUVs.u1, skullUVs.v1);
    }

    renderHUD();

    if (m_state == GameState::GameOver)
        renderEndScreen("GAME OVER", { 0.9f, 0.2f, 0.2f, 1.f });
    else if (m_state == GameState::Win)
        renderEndScreen("YOU WIN!", { 0.2f, 0.9f, 0.2f, 1.f });
}
