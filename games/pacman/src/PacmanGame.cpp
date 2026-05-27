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

    m_pacTex   = std::make_shared<Engine::Texture>("games/pacman/assets/sprites/PacManAssets-PacMan.png");
    m_pacSheet = std::make_shared<Engine::SpriteSheet>(m_pacTex, kPacSheetCols, kPacSheetRows);

    buildDotCache();

    // Construct Pac-Man after map and spritesheet are ready
    const auto* wallLayer = m_map.findLayer("Wall");
    if (wallLayer)
        m_pacman.emplace(*wallLayer, m_pacSheet);
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

void PacmanGame::onUpdate(float dt) {
    if (Engine::Input::isKeyPressed(GLFW_KEY_Q))
        quit();

    if (m_pacman)
        m_pacman->update(dt);
}

void PacmanGame::onRender() {
    m_renderer.beginScene(WIN_W, WIN_H);
    renderWalls();
    renderDots();
    if (m_pacman)
        m_pacman->render(m_renderer, kLayerPacman);
}

void PacmanGame::renderWalls() {
    const auto* layer = m_map.findLayer("Wall");
    const auto* ts    = m_map.tilesetForGid(1);
    if (!layer || !ts) return;

    Engine::Tilemap::renderLayer(m_renderer, *layer, *m_wallSheet, *ts,
                                 static_cast<float>(TILE * SCALE), kLayerWalls);
}

void PacmanGame::renderDots() {
    const auto* layer = m_map.findLayer("Dots");
    if (!layer) return;

    const float tileSize = static_cast<float>(TILE * SCALE);
    for (int i = 0; i < static_cast<int>(m_dots.size()); ++i) {
        if (m_dots[i] == CellType::Empty) continue;
        int frameId = (m_dots[i] == CellType::Dot) ? kDotFrameId : kPelletFrameId;
        auto uv = m_itemsSheet->getFrameUVs(frameId);
        int   col = i % layer->cols;
        int   row = i / layer->cols;
        float x   = col * tileSize;
        float y   = row * tileSize;
        m_renderer.drawTexturedRect(x, y, tileSize, tileSize,
                                    m_itemsSheet->texture(),
                                    uv.u0, uv.v0, uv.u1, uv.v1,
                                    0.f, {1.f, 1.f, 1.f, 1.f}, kLayerDots);
    }
}
