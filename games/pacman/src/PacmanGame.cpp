#include "PacmanGame.h"
#include "GameConstants.h"
#include <engine/renderer/PixelFont.h>
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

    m_ghostTex   = std::make_shared<Engine::Texture>("games/pacman/assets/sprites/PacManAssets-Ghosts.png");
    m_ghostSheet = std::make_shared<Engine::SpriteSheet>(m_ghostTex, kGhostSheetCols, kGhostSheetRows);

    buildDotCache();

    // Resolve spawn position from object layer, fall back to classic Pac-Man start
    int spawnCol = 14;
    int spawnRow = 23;
    if (const auto* spawn = m_map.findObject("PacmanSpawn")) {
        spawnCol = static_cast<int>(spawn->x) / m_map.tileWidth;
        spawnRow = static_cast<int>(spawn->y) / m_map.tileHeight;
    }

    // Construct Pac-Man and ghosts after map and spritesheets are ready
    const auto* wallLayer = m_map.findLayer("Wall");
    if (wallLayer) {
        m_pacman.emplace(*wallLayer, m_pacSheet, spawnCol, spawnRow);

        // All four ghosts start just above the ghost house (col 14, row 11).
        // Spawn positions and exit logic are added in a later phase.
        m_ghosts.reserve(4);
        m_ghosts.emplace_back(*wallLayer, m_ghostSheet, GhostType::Blinky, 14, 11);
        m_ghosts.emplace_back(*wallLayer, m_ghostSheet, GhostType::Pinky,  14, 11);
        m_ghosts.emplace_back(*wallLayer, m_ghostSheet, GhostType::Inky,   14, 11);
        m_ghosts.emplace_back(*wallLayer, m_ghostSheet, GhostType::Clyde,  14, 11);
    }
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

// Level 1 scatter/chase schedule in seconds: Scatter 7, Chase 20, Scatter 7,
// Chase 20, Scatter 5, Chase 20, Scatter 5, then Chase permanently.
static constexpr float kModeSchedule[] = { 7.f, 20.f, 7.f, 20.f, 5.f, 20.f, 5.f };
static constexpr int   kModeCount      = static_cast<int>(std::size(kModeSchedule));

void PacmanGame::updateModeTimer(float dt) {
    m_modeTimer -= dt;
    if (m_modeTimer > 0.f) return;

    ++m_modePhase;
    m_currentMode = (m_modePhase % 2 == 0) ? GhostMode::Scatter : GhostMode::Chase;
    m_modeTimer   = (m_modePhase < kModeCount) ? kModeSchedule[m_modePhase] : 1e9f;

    for (auto& ghost : m_ghosts)
        ghost.setMode(m_currentMode);
}

void PacmanGame::tryEatDot() {
    if (!m_pacman) return;
    const auto* layer = m_map.findLayer("Dots");
    if (!layer) return;

    int idx = m_pacman->row() * layer->cols + m_pacman->col();
    if (idx < 0 || idx >= static_cast<int>(m_dots.size())) return;

    switch (m_dots[idx]) {
        case CellType::Dot:
            m_dots[idx] = CellType::Empty;
            m_score += kScoreDot;
            break;
        case CellType::PowerPellet:
            m_dots[idx] = CellType::Empty;
            m_score += kScorePellet;
            break;
        default: break;
    }
}

void PacmanGame::onUpdate(float dt) {
    if (Engine::Input::isKeyPressed(GLFW_KEY_Q))
        quit();

    if (m_pacman)
        m_pacman->update(dt);

    updateModeTimer(dt);

    for (auto& ghost : m_ghosts)
        ghost.update(dt);

    tryEatDot();
}

void PacmanGame::onRender() {
    m_renderer.beginScene(WIN_W, WIN_H);
    renderWalls();
    renderDots();
    renderGhosts();
    if (m_pacman)
        m_pacman->render(m_renderer, kLayerPacman);
    renderHUD();
#ifdef ENABLE_DEV_KEYS
    renderDevHUD();
#endif
}

void PacmanGame::renderWalls() {
    const auto* layer = m_map.findLayer("Wall");
    const auto* ts    = m_map.tilesetForGid(1);
    if (!layer || !ts) return;

    Engine::Tilemap::renderLayer(m_renderer, *layer, *m_wallSheet, *ts,
                                 static_cast<float>(TILE * SCALE), kLayerWalls);
}

void PacmanGame::renderGhosts() {
    for (const auto& ghost : m_ghosts)
        ghost.render(m_renderer, kLayerGhosts);
}

void PacmanGame::renderDevHUD() {
    constexpr float kScale  = 1.5f;
    constexpr float kX      = 8.f;
    constexpr float kY      = WIN_H - 32.f;
    constexpr glm::vec4 kScatterCol = {0.4f, 0.8f, 1.f, 1.f}; // cyan
    constexpr glm::vec4 kChaseCol   = {1.f, 0.4f, 0.4f, 1.f}; // red

    const bool scatter = (m_currentMode == GhostMode::Scatter);
    const glm::vec4& col = scatter ? kScatterCol : kChaseCol;
    const std::string label = scatter ? "SCATTER" : "CHASE";

    Engine::PixelFont::drawString(m_renderer, label, kX, kY, kScale, col, kLayerHUD);

    // Timer countdown — show tenths of a second
    const std::string timer = std::to_string(static_cast<int>(m_modeTimer))
                            + "." + std::to_string(static_cast<int>(m_modeTimer * 10.f) % 10);
    const float timerX = kX + Engine::PixelFont::stringWidth(label, kScale) + 8.f;
    Engine::PixelFont::drawString(m_renderer, timer, timerX, kY, kScale, {0.8f, 0.8f, 0.8f, 1.f}, kLayerHUD);
}

void PacmanGame::renderHUD() {
    constexpr float kFontScale = 2.f;
    constexpr float kMargin    = 8.f;
    Engine::PixelFont::drawString(m_renderer, std::to_string(m_score),
                                  kMargin, kMargin, kFontScale,
                                  {1.f, 1.f, 1.f, 1.f}, kLayerHUD);
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
