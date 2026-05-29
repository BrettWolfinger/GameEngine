#include "MarioGame.h"
#include "GameConstants.h"
#include <GLFW/glfw3.h>
#include <algorithm> // std::clamp

static constexpr int kLayerBackground = 0;
static constexpr int kLayerTerrain    = 2;
static constexpr int kLayerPlayer     = 3;


// Tileset sheet: 20 cols × 20 rows
static constexpr int kTileSheetCols = 20;
static constexpr int kTileSheetRows = 20;

MarioGame::MarioGame()
    : Engine::Application("Super Mario Bros", WIN_W, WIN_H)
{}

void MarioGame::onInit() {
    m_map = Engine::Tilemap::loadMap("games/mario/assets/maps/1_1map.tmx");

    const auto* ts = m_map.tilesetForGid(1);
    m_tilesetTex   = std::make_shared<Engine::Texture>(ts->imagePath);
    m_tilesetSheet = std::make_shared<Engine::SpriteSheet>(m_tilesetTex, kTileSheetCols, kTileSheetRows);

    m_marioTex   = std::make_shared<Engine::Texture>("games/mario/assets/sprites/smb-mario.png");
    m_marioSheet = std::make_shared<Engine::SpriteSheet>(m_marioTex, Engine::FrameSize{ MARIO_FRAME_W, MARIO_FRAME_H });

    const float startX = 3.f * TILE * SCALE;
    const float startY = static_cast<float>((SCREEN_ROWS - 2) * TILE * SCALE - MARIO_FRAME_H * SCALE);
    m_player = std::make_unique<Player>(m_marioSheet, startX, startY);
}

void MarioGame::onUpdate(float dt) {
    if (Engine::Input::isKeyPressed(GLFW_KEY_Q))
        quit();

    m_player->update(dt);

    const float playerCenterX = m_player->x() + MARIO_FRAME_W * 0.5f * SCALE;
    const float mapWidth      = static_cast<float>(m_map.cols * TILE * SCALE);
    m_cameraX = std::clamp(playerCenterX - WIN_W * 0.5f, 0.f, mapWidth - WIN_W);
}

void MarioGame::onRender() {
    m_renderer.beginScene(WIN_W, WIN_H);
    renderBackground();
    renderTerrain();
    renderPlayer();
}

void MarioGame::renderBackground() {
    const Engine::Tilemap::Color4& c = m_map.backgroundColor;
    m_renderer.drawRect(0.f, 0.f, static_cast<float>(WIN_W), static_cast<float>(WIN_H),
                        { c.r, c.g, c.b, c.a }, kLayerBackground);
}

void MarioGame::renderTerrain() {
    const auto* layer = m_map.findLayer("Terrain");
    const auto* ts    = m_map.tilesetForGid(1);
    if (!layer || !ts) return;

    Engine::Tilemap::renderLayer(m_renderer, *layer, *m_tilesetSheet, *ts,
                                 static_cast<float>(TILE * SCALE), kLayerTerrain,
                                 m_cameraX, 0.f,
                                 static_cast<float>(WIN_W), static_cast<float>(WIN_H));
}

void MarioGame::renderPlayer() {
    m_player->render(m_renderer, m_cameraX, kLayerPlayer);
}
