#include "MarioGame.h"
#include "GameConstants.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

// Tileset sheet: 20 cols × 20 rows, 320×320px
static constexpr int kSheetCols = 20;
static constexpr int kSheetRows = 20;

// Render layers
static constexpr int kLayerBackground = 0;
static constexpr int kLayerDeco       = 1;
static constexpr int kLayerTerrain    = 2;


MarioGame::MarioGame()
    : Engine::Application("Super Mario Bros", WIN_W, WIN_H)
{}

void MarioGame::onInit() {
    m_tilesetTex   = std::make_shared<Engine::Texture>("games/mario/assets/maps/smb1_1tileset.png");
    m_tilesetSheet = std::make_shared<Engine::SpriteSheet>(m_tilesetTex, kSheetCols, kSheetRows);

    m_map = loadMap("games/mario/assets/maps/1_1map.tmx");
}

void MarioGame::onUpdate(float /*dt*/) {
    if (Engine::Input::isKeyPressed(GLFW_KEY_Q))
        quit();
}

void MarioGame::onRender() {
    m_renderer.beginScene(WIN_W, WIN_H);
    renderBackground();
    renderTerrain();
}

void MarioGame::renderBackground() {
    const Color4& c = m_map.backgroundColor;
    m_renderer.drawRect(0.f, 0.f, static_cast<float>(WIN_W), static_cast<float>(WIN_H),
                        { c.r, c.g, c.b, c.a }, kLayerBackground);
}

void MarioGame::renderTerrain() {
    const TileLayer* layer = m_map.findLayer("Terrain");
    if (!layer) return;

    const float ts = static_cast<float>(TILE * SCALE);

    // Cull to visible column range only
    int firstCol = static_cast<int>(m_cameraX / ts);
    int lastCol  = static_cast<int>((m_cameraX + WIN_W) / ts) + 1;
    firstCol = std::max(firstCol, 0);
    lastCol  = std::min(lastCol, layer->cols);

    for (int row = 0; row < layer->rows; ++row) {
        for (int col = firstCol; col < lastCol; ++col) {
            uint32_t gid = layer->gids[row * layer->cols + col];
            if (gid == 0) continue;

            const TilesetRef* ts_ref = m_map.tilesetForGid(gid);
            if (!ts_ref) continue;

            int localId = static_cast<int>(gid) - ts_ref->firstGid;
            auto uv = m_tilesetSheet->getFrameUVs(localId);

            float screenX = col * ts - m_cameraX;
            float screenY = row * ts;

            m_renderer.drawTexturedRect(
                screenX, screenY, ts, ts,
                m_tilesetSheet->texture(),
                uv.u0, uv.v0, uv.u1, uv.v1,
                0.f, {1.f, 1.f, 1.f, 1.f},
                kLayerTerrain
            );
        }
    }
}
