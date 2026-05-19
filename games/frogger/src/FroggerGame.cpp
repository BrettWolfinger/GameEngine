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

    // River zone: frog must be on a platform or it drowns
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

    for (int i = 0; i < HOME_SLOT_COUNT; ++i)
        m_renderer.drawRect(static_cast<float>(HOME_SLOTS[i] * TILE), 0.f,
                            static_cast<float>(TILE), static_cast<float>(TILE), goal);
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
