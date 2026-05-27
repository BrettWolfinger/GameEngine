#include "PacmanGame.h"
#include "GameConstants.h"
#include <GLFW/glfw3.h>

// Items tileset: dot = local ID 8, power pellet = local ID 9
static constexpr int kDotFrameId    = 8;
static constexpr int kPelletFrameId = 9;

// Items firstgid as declared in the TMX
static constexpr int kItemsFirstGid = 248;

PacmanGame::PacmanGame()
    : Engine::Application("Pac-Man", WIN_W, WIN_H)
{}

void PacmanGame::onInit() {
    m_map = Engine::Tilemap::loadMap("games/pacman/assets/maps/PacManMap.tmx");

    // Load tilesets — columns come from the TSX via the engine loader
    const auto* wallTs  = m_map.tilesetForGid(1);
    const auto* itemsTs = m_map.tilesetForGid(kItemsFirstGid);

    m_wallTex   = std::make_shared<Engine::Texture>(wallTs->imagePath);
    m_wallSheet = std::make_shared<Engine::SpriteSheet>(m_wallTex, wallTs->columns,
                                                        wallTs->tileCount / wallTs->columns);

    m_itemsTex   = std::make_shared<Engine::Texture>(itemsTs->imagePath);
    m_itemsSheet = std::make_shared<Engine::SpriteSheet>(m_itemsTex, itemsTs->columns,
                                                          itemsTs->tileCount / itemsTs->columns);
    buildDotCache();
}

void PacmanGame::buildDotCache() {
    const auto* layer = m_map.findLayer("Dots");
    if (!layer) return;

    m_dots.resize(layer->gids.size(), CellType::Empty);
    for (size_t i = 0; i < layer->gids.size(); ++i) {
        int localId = static_cast<int>(Engine::Tilemap::stripFlips(layer->gids[i])) - kItemsFirstGid;
        if      (localId == kDotFrameId)    m_dots[i] = CellType::Dot;
        else if (localId == kPelletFrameId) m_dots[i] = CellType::PowerPellet;
    }
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
    const auto* layer = m_map.findLayer("Wall");
    if (!layer) return;

    const auto* ts = m_map.tilesetForGid(1);
    const float tileSize = static_cast<float>(TILE * SCALE);

    for (int row = 0; row < layer->rows; ++row) {
        for (int col = 0; col < layer->cols; ++col) {
            uint32_t raw = layer->gids[row * layer->cols + col];
            if (Engine::Tilemap::stripFlips(raw) == 0) continue;

            int localId = static_cast<int>(Engine::Tilemap::stripFlips(raw)) - ts->firstGid;
            auto flipped = Engine::Tilemap::applyFlips(m_wallSheet->getFrameUVs(localId), raw);

            m_renderer.drawTexturedRect(
                col * tileSize, row * tileSize, tileSize, tileSize,
                m_wallSheet->texture(),
                flipped.uv.u0, flipped.uv.v0, flipped.uv.u1, flipped.uv.v1,
                flipped.angle, {1.f, 1.f, 1.f, 1.f},
                kLayerWalls
            );
        }
    }
}

void PacmanGame::renderDots() {
    const auto* layer = m_map.findLayer("Dots");
    if (!layer) return;

    const float tileSize = static_cast<float>(TILE * SCALE);

    for (int row = 0; row < layer->rows; ++row) {
        for (int col = 0; col < layer->cols; ++col) {
            CellType ct = m_dots[row * layer->cols + col];
            if (ct == CellType::Empty) continue;

            int frameId = (ct == CellType::Dot) ? kDotFrameId : kPelletFrameId;
            auto uv = m_itemsSheet->getFrameUVs(frameId);

            m_renderer.drawTexturedRect(
                col * tileSize, row * tileSize, tileSize, tileSize,
                m_itemsSheet->texture(),
                uv.u0, uv.v0, uv.u1, uv.v1,
                0.f, {1.f, 1.f, 1.f, 1.f},
                kLayerDots
            );
        }
    }
}
