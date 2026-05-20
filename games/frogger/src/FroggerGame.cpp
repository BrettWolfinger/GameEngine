#include "FroggerGame.h"
#include "FroggerConfig.h"
#include <engine/renderer/Texture.h>
#include <engine/core/Input.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/constants.hpp>

static constexpr int HOME_FILLED_FRAME = 4;  // row 0 col 4 (0-indexed)

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

void FroggerGame::onUpdate(float dt) {
    if (Engine::Input::isKeyPressed(GLFW_KEY_Q))
        quit();

    m_frog->update(dt);

#ifdef ENABLE_DEV_KEYS
    if (Engine::Input::isKeyPressed(GLFW_KEY_F1))
        m_frog->teleport(6, MEDIAN_ROW);
#endif

    for (auto& v : m_vehicles)
        v.update(dt);
    for (auto& p : m_platforms)
        p.update(dt);

    const int   frogRow = m_frog->row();
    const float frogPx  = m_frog->pixelX();

    // Home row: land on a valid unfilled slot or die
    if (frogRow == HOME_ROW) {
        int slotIdx = -1;
        for (int i = 0; i < HOME_SLOT_COUNT; ++i) {
            if (m_frog->col() == HOME_SLOTS[i]) { slotIdx = i; break; }
        }
        if (slotIdx >= 0 && !m_filledSlots[slotIdx]) {
            m_filledSlots[slotIdx] = true;
            m_frog->reset();
            m_allHomesFilled = true;
            for (int i = 0; i < HOME_SLOT_COUNT; ++i)
                if (!m_filledSlots[i]) { m_allHomesFilled = false; break; }
        } else {
            m_frog->reset();
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
                m_frog->reset();
        } else {
            m_frog->reset();
        }
    }

    // Road zone: vehicle collision resets frog
    if (frogRow >= ROAD_FIRST_ROW && frogRow <= ROAD_LAST_ROW) {
        for (const auto& v : m_vehicles) {
            if (v.row() != frogRow) continue;
            const float vw = static_cast<float>(v.tileWidth() * TILE);
            if (frogPx < v.x() + vw && frogPx + TILE > v.x()) {
                m_frog->reset();
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

    // Empty home slots
    for (int i = 0; i < HOME_SLOT_COUNT; ++i)
        m_renderer.drawRect(static_cast<float>(HOME_SLOTS[i] * TILE), 0.f,
                            static_cast<float>(TILE), static_cast<float>(TILE), goal);

    // Filled home slots — sprite from sheet
    const Engine::UVRect uvs = m_sheet->getFrameUVs(HOME_FILLED_FRAME);
    for (int i = 0; i < HOME_SLOT_COUNT; ++i) {
        if (!m_filledSlots[i]) continue;
        m_renderer.drawTexturedRect(static_cast<float>(HOME_SLOTS[i] * TILE), 0.f,
                                    static_cast<float>(TILE), static_cast<float>(TILE),
                                    m_sheet->texture(),
                                    uvs.u0, uvs.v0, uvs.u1, uvs.v1,
                                    glm::pi<float>());
    }
}

void FroggerGame::onRender() {
    m_renderer.beginScene(W, H);
    renderBackground();

    for (auto& p : m_platforms)
        p.render(m_renderer, *m_sheet);
    for (auto& v : m_vehicles)
        v.render(m_renderer, *m_sheet);

    m_frog->render(m_renderer);
}
