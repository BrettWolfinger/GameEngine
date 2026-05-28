#include "PacmanGame.h"
#include "GameConstants.h"
#include <engine/renderer/PixelFont.h>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <fstream>

// Items tileset: dot = local ID 8, power pellet = local ID 9
static constexpr int kDotFrameId    = 8;
static constexpr int kPelletFrameId = 9;

// Items firstgid as declared in the TMX
static constexpr int kItemsFirstGid = 248;

PacmanGame::PacmanGame()
    : Engine::Application("Pac-Man", WIN_W, WIN_H)
{}

void PacmanGame::onInit() {
    registerConfig("games/pacman/assets/configs/pacman.toml",     &m_config);
    registerConfig("games/pacman/assets/configs/pacman_hud.toml", &m_hudConfig);
    m_lives = m_config.startLives;
    loadHighScore();
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
    m_dotsLayer = m_map.findLayer("Dots");
    if (!m_dotsLayer) return;

    m_dots.assign(m_dotsLayer->gids.size(), CellType::Empty);
    m_dotsRemaining = 0;
    for (size_t i = 0; i < m_dotsLayer->gids.size(); ++i) {
        int localId = static_cast<int>(Engine::Tilemap::stripFlips(m_dotsLayer->gids[i])) - kItemsFirstGid;
        if      (localId == kDotFrameId)    { m_dots[i] = CellType::Dot;         ++m_dotsRemaining; }
        else if (localId == kPelletFrameId) { m_dots[i] = CellType::PowerPellet; ++m_dotsRemaining; }
    }
}

void PacmanGame::updateModeTimer(float dt) {
    // Mode timer is paused while ghosts are frightened.
    if (m_frightenedTimer > 0.f) return;

    m_modeTimer -= dt;
    if (m_modeTimer > 0.f) return;

    ++m_modePhase;
    m_currentMode = (m_modePhase % 2 == 0) ? GhostMode::Scatter : GhostMode::Chase;
    m_modeTimer   = (m_modePhase < kModeCount) ? kModeSchedule[m_modePhase] : 1e9f;

    for (auto& ghost : m_ghosts)
        ghost.setMode(m_currentMode);
}

void PacmanGame::triggerFrightened() {
    m_frightenedTimer       = m_config.frightenedDuration;
    m_ghostsEatenThisPellet = 0;
    for (auto& ghost : m_ghosts)
        ghost.frighten();
}

void PacmanGame::updateFrightenedTimer(float dt) {
    if (m_frightenedTimer <= 0.f) return;
    m_frightenedTimer -= dt;
    if (m_frightenedTimer <= 0.f) {
        m_frightenedTimer = 0.f;
        for (auto& ghost : m_ghosts)
            ghost.endFrightened(m_currentMode);
    }
}

void PacmanGame::checkGhostCollision() {
    if (!m_pacman || m_gameState != GameState::Playing) return;
    const int pc = m_pacman->col();
    const int pr = m_pacman->row();

    for (auto& ghost : m_ghosts) {
        if (ghost.col() != pc || ghost.row() != pr) continue;

        if (ghost.mode() == GhostMode::Frightened) {
            // Eat the ghost — score doubles per ghost eaten this pellet.
            m_ghostsEatenThisPellet++;
            m_score += kGhostScoreBase << (m_ghostsEatenThisPellet - 1);
            updateHighScore();
            ghost.respawn();
        } else if (ghost.mode() != GhostMode::Eyes) {
            // Pac-Man is caught — begin death sequence.
            startDeathSequence();
            return; // stop checking remaining ghosts
        }
    }
}

void PacmanGame::startDeathSequence() {
    m_gameState       = GameState::Dying;
    m_deathPauseTimer = m_config.deathPause;
    m_frightenedTimer = 0.f;
    for (auto& ghost : m_ghosts)
        ghost.endFrightened(m_currentMode); // snap all ghosts out of frightened
    if (m_pacman) m_pacman->startDeath();
}

void PacmanGame::handleDyingState(float dt) {
    // Phase 1: death animation still playing.
    if (m_pacman && !m_pacman->isDeathDone()) {
        m_pacman->update(dt, 0.f);
        return;
    }

    // Phase 2: animation done — wait out the pause before respawning.
    if (m_deathPauseTimer > 0.f) {
        m_deathPauseTimer -= dt;
        return;
    }

    // Phase 3: consume a life then respawn or end the game.
    --m_lives;
    if (m_lives <= 0) {
        m_gameState = GameState::GameOver;
        return;
    }
    respawnAfterDeath();
}

void PacmanGame::resetModeSchedule() {
    m_modePhase             = 0;
    m_modeTimer             = kModeSchedule[0];
    m_currentMode           = GhostMode::Scatter;
    m_frightenedTimer       = 0.f;
    m_ghostsEatenThisPellet = 0;
}

void PacmanGame::respawnAfterDeath() {
    resetModeSchedule();
    m_gameState = GameState::Playing;
    if (m_pacman) m_pacman->respawn();
    for (auto& ghost : m_ghosts) ghost.respawn();
}

void PacmanGame::startLevelClear() {
    m_gameState       = GameState::LevelClear;
    m_levelClearTimer = m_config.levelClearPause;
    m_frightenedTimer = 0.f;
    for (auto& ghost : m_ghosts)
        ghost.endFrightened(m_currentMode);
}

void PacmanGame::startNextLevel() {
    // Score and lives carry over; everything else resets.
    resetModeSchedule();
    m_gameState = GameState::Playing;
    buildDotCache();
    if (m_pacman) m_pacman->respawn();
    for (auto& ghost : m_ghosts) ghost.respawn();
}

void PacmanGame::restartGame() {
    m_score  = 0;
    m_lives  = m_config.startLives;
    resetModeSchedule();
    m_gameState = GameState::Playing;
    buildDotCache();
    if (m_pacman) m_pacman->respawn();
    for (auto& ghost : m_ghosts) ghost.respawn();
}

void PacmanGame::updateHighScore() {
    if (m_score > m_highScore) {
        m_highScore = m_score;
        saveHighScore();
    }
}

void PacmanGame::loadHighScore() {
    std::ifstream f("games/pacman/highscore.dat");
    if (f) f >> m_highScore;
}

void PacmanGame::saveHighScore() {
    std::ofstream f("games/pacman/highscore.dat");
    if (f) f << m_highScore;
}

void PacmanGame::tryEatDot() {
    if (!m_pacman || !m_dotsLayer) return;

    int idx = m_pacman->row() * m_dotsLayer->cols + m_pacman->col();
    if (idx < 0 || idx >= static_cast<int>(m_dots.size())) return;

    switch (m_dots[idx]) {
        case CellType::Dot:
            m_dots[idx] = CellType::Empty;
            m_score += kScoreDot;
            --m_dotsRemaining;
            break;
        case CellType::PowerPellet:
            m_dots[idx] = CellType::Empty;
            m_score += kScorePellet;
            --m_dotsRemaining;
            triggerFrightened();
            break;
        default: break;
    }

    updateHighScore();

    if (m_dotsRemaining == 0)
        startLevelClear();
}

void PacmanGame::onUpdate(float dt) {
    if (Engine::Input::isKeyPressed(GLFW_KEY_Q))
        quit();

    if (m_gameState == GameState::GameOver) {
        if (Engine::Input::isKeyPressed(GLFW_KEY_R))
            restartGame();
        return;
    }

    if (m_gameState == GameState::LevelClear) {
        m_levelClearTimer -= dt;
        if (m_levelClearTimer <= 0.f)
            startNextLevel();
        return;
    }

    if (m_gameState == GameState::Dying) {
        handleDyingState(dt);
        return;
    }

    // --- Normal Playing state ---
    updateModeTimer(dt);
    updateFrightenedTimer(dt);

    if (m_pacman)
        m_pacman->update(dt, m_config.pacmanSpeed);

    if (!m_ghosts.empty() && m_pacman) {
        const int  blinkyCol = m_ghosts[0].col();
        const int  blinkyRow = m_ghosts[0].row();
        const int  pacCol    = m_pacman->col();
        const int  pacRow    = m_pacman->row();
        const Dir  pacDir    = m_pacman->dir();
        for (auto& ghost : m_ghosts)
            ghost.update(dt, pacCol, pacRow, pacDir, blinkyCol, blinkyRow,
                         m_config.ghostSpeed, m_config.ghostSpeedFrightened);
    }

    tryEatDot();
    checkGhostCollision();
}

void PacmanGame::onRender() {
    m_renderer.beginScene(WIN_W, WIN_H);
    renderWalls();
    renderDots();
    renderGhosts();
    if (m_pacman)
        m_pacman->render(m_renderer, kLayerPacman, static_cast<float>(HUD_H));
    renderHUD();
#ifdef ENABLE_DEV_KEYS
    renderDevHUD();
#endif
}

void PacmanGame::renderWalls() {
    const auto* layer = m_map.findLayer("Wall");
    const auto* ts    = m_map.tilesetForGid(1);
    if (!layer || !ts) return;

    // cameraY = -HUD_H shifts the world down by the HUD strip height.
    Engine::Tilemap::renderLayer(m_renderer, *layer, *m_wallSheet, *ts,
                                 static_cast<float>(TILE * SCALE), kLayerWalls,
                                 0.f, -static_cast<float>(HUD_H));
}

void PacmanGame::renderGhosts() {
    for (const auto& ghost : m_ghosts)
        ghost.render(m_renderer, kLayerGhosts, static_cast<float>(HUD_H));
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
    // Score — top-left
    Engine::PixelFont::drawString(m_renderer, std::to_string(m_score),
                                  m_hudConfig.margin, m_hudConfig.margin,
                                  m_hudConfig.fontScale,
                                  {1.f, 1.f, 1.f, 1.f}, kLayerHUD);

    // High score — top-center (label row + value row, 1px gap)
    const std::string hiLabel = "HI";
    const std::string hiValue = std::to_string(m_highScore);
    const float labelW   = Engine::PixelFont::stringWidth(hiLabel, m_hudConfig.fontScale);
    const float valueW   = Engine::PixelFont::stringWidth(hiValue, m_hudConfig.fontScale);
    const float hiBlockW = std::max(labelW, valueW);
    const float hiX      = (WIN_W - hiBlockW) * 0.5f;
    Engine::PixelFont::drawString(m_renderer, hiLabel,
                                  hiX + (hiBlockW - labelW) * 0.5f, m_hudConfig.margin,
                                  m_hudConfig.fontScale, {1.f, 0.8f, 0.f, 1.f}, kLayerHUD);
    Engine::PixelFont::drawString(m_renderer, hiValue,
                                  hiX + (hiBlockW - valueW) * 0.5f,
                                  m_hudConfig.margin + 7.f * m_hudConfig.fontScale + 1.f,
                                  m_hudConfig.fontScale, {1.f, 1.f, 1.f, 1.f}, kLayerHUD);

    renderLives();
    renderOverlay();
}

void PacmanGame::renderOverlay() {
    if (m_gameState == GameState::LevelClear) {
        constexpr float kScale = 3.f;
        const std::string text = "LEVEL CLEAR";
        const float w = Engine::PixelFont::stringWidth(text, kScale);
        Engine::PixelFont::drawString(m_renderer, text,
                                      (WIN_W - w) * 0.5f, WIN_H * 0.5f - 12.f,
                                      kScale, {1.f, 1.f, 0.f, 1.f}, kLayerHUD);
        return;
    }

    if (m_gameState == GameState::GameOver) {
        constexpr float kScale     = 3.f;
        constexpr float kHintScale = 1.5f;
        const std::string title = "GAME OVER";
        const std::string hint  = "R TO RESTART";
        const float tw = Engine::PixelFont::stringWidth(title, kScale);
        const float hw = Engine::PixelFont::stringWidth(hint, kHintScale);
        Engine::PixelFont::drawString(m_renderer, title,
                                      (WIN_W - tw) * 0.5f, WIN_H * 0.5f - 16.f,
                                      kScale, {1.f, 0.f, 0.f, 1.f}, kLayerHUD);
        Engine::PixelFont::drawString(m_renderer, hint,
                                      (WIN_W - hw) * 0.5f, WIN_H * 0.5f + 12.f,
                                      kHintScale, {0.8f, 0.8f, 0.8f, 1.f}, kLayerHUD);
    }
}

void PacmanGame::renderLives() {
    // Draw one Pac-Man icon per spare life (lives - 1; current life not shown as an icon).
    const int spares = std::max(0, m_lives - 1);
    const auto uv = m_pacSheet->getFrameUVs(1); // frame 1: half-open right-facing Pac-Man

    for (int i = 0; i < spares; ++i) {
        // Pack icons right-to-left from the right edge
        const float x = WIN_W - m_hudConfig.margin - (i + 1) * (m_hudConfig.iconSize + 4.f) + 4.f;
        m_renderer.drawTexturedRect(x, m_hudConfig.margin, m_hudConfig.iconSize, m_hudConfig.iconSize,
                                    m_pacSheet->texture(),
                                    uv.u0, uv.v0, uv.u1, uv.v1,
                                    0.f, {1.f, 1.f, 1.f, 1.f}, kLayerHUD);
    }
}

void PacmanGame::renderDots() {
    if (!m_dotsLayer) return;

    const float tileSize = static_cast<float>(TILE * SCALE);
    for (int i = 0; i < static_cast<int>(m_dots.size()); ++i) {
        if (m_dots[i] == CellType::Empty) continue;
        int frameId = (m_dots[i] == CellType::Dot) ? kDotFrameId : kPelletFrameId;
        auto uv = m_itemsSheet->getFrameUVs(frameId);
        int   col = i % m_dotsLayer->cols;
        int   row = i / m_dotsLayer->cols;
        float x   = col * tileSize;
        float y   = row * tileSize + static_cast<float>(HUD_H);
        m_renderer.drawTexturedRect(x, y, tileSize, tileSize,
                                    m_itemsSheet->texture(),
                                    uv.u0, uv.v0, uv.u1, uv.v1,
                                    0.f, {1.f, 1.f, 1.f, 1.f}, kLayerDots);
    }
}
