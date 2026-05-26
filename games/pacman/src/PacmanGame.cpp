#include "PacmanGame.h"
#include "MapLoader.h"
#include "GameConstants.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

static constexpr int kWallCols    = 19;
static constexpr int kWallRows    = 13;
static constexpr int kItemsCols   = 8;
static constexpr int kItemsRows   = 2;
static constexpr int kWallFirstGid   = 1;
static constexpr int kDotFrameId     = 8;  // items row 1, col 0
static constexpr int kPelletFrameId  = 9;  // items row 1, col 1

PacmanGame::PacmanGame()
    : Engine::Application("Pac-Man", WIN_W, WIN_H)
{}

void PacmanGame::onInit() {
    m_wallTex   = std::make_shared<Engine::Texture>("games/pacman/assets/sprites/PacManAssets_Map_TileSet.png");
    m_wallSheet = std::make_shared<Engine::SpriteSheet>(m_wallTex, kWallCols, kWallRows);

    m_itemsTex   = std::make_shared<Engine::Texture>("games/pacman/assets/sprites/PacManAssets-Items.png");
    m_itemsSheet = std::make_shared<Engine::SpriteSheet>(m_itemsTex, kItemsCols, kItemsRows);

    m_map = loadTmx("games/pacman/assets/maps/PacManMap.tmx");
}

void PacmanGame::onUpdate(float /*dt*/) {
    if (Engine::Input::isKeyPressed(GLFW_KEY_Q))
        quit();
}

void PacmanGame::onRender() {
    m_renderer.beginScene(WIN_W, WIN_H);
    renderWalls();
    renderDots();
}

void PacmanGame::renderWalls() {
    static constexpr float kHalfPi = 1.5707963268f;
    const float ts = static_cast<float>(TILE * SCALE);

    for (int row = 0; row < m_map.rows; ++row) {
        for (int col = 0; col < m_map.cols; ++col) {
            const WallTile& cell = m_map.walls[row * m_map.cols + col];
            if (cell.gid == 0) continue;

            int localId = static_cast<int>(cell.gid) - kWallFirstGid;
            auto uv = m_wallSheet->getFrameUVs(localId);

            float ru0 = uv.u0, rv0 = uv.v0, ru1 = uv.u1, rv1 = uv.v1;
            float angle = 0.f;

            if (!cell.flipD) {
                if (cell.flipH) std::swap(ru0, ru1);
                if (cell.flipV) std::swap(rv0, rv1);
            } else if (cell.flipH && !cell.flipV) {
                angle = +kHalfPi;                      // 90° CW
            } else if (!cell.flipH && cell.flipV) {
                angle = -kHalfPi;                      // 90° CCW
            } else if (!cell.flipH && !cell.flipV) {
                angle = -kHalfPi; std::swap(ru0, ru1); // main diagonal flip
            } else {
                angle = +kHalfPi; std::swap(ru0, ru1); // anti-diagonal flip
            }

            m_renderer.drawTexturedRect(
                col * ts, row * ts, ts, ts,
                m_wallSheet->texture(),
                ru0, rv0, ru1, rv1,
                angle, {1.f, 1.f, 1.f, 1.f},
                kLayerWalls
            );
        }
    }
}

void PacmanGame::renderDots() {
    const float ts = static_cast<float>(TILE * SCALE);
    for (int row = 0; row < m_map.rows; ++row) {
        for (int col = 0; col < m_map.cols; ++col) {
            CellType ct = m_map.dots[row * m_map.cols + col];
            if (ct == CellType::Empty) continue;

            int frameId = (ct == CellType::Dot) ? kDotFrameId : kPelletFrameId;
            auto uv = m_itemsSheet->getFrameUVs(frameId);

            m_renderer.drawTexturedRect(
                col * ts, row * ts, ts, ts,
                m_itemsSheet->texture(),
                uv.u0, uv.v0, uv.u1, uv.v1,
                0.f, {1.f, 1.f, 1.f, 1.f},
                kLayerDots
            );
        }
    }
}
