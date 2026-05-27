#include "MarioGame.h"
#include "GameConstants.h"
#include <GLFW/glfw3.h>
#include <algorithm> // std::clamp

// Render layers
static constexpr int kLayerBackground = 0;
static constexpr int kLayerDeco       = 1;
static constexpr int kLayerTerrain    = 2;

// Tileset sheet: 20 cols × 20 rows
static constexpr int kSheetCols = 20;
static constexpr int kSheetRows = 20;

MarioGame::MarioGame()
    : Engine::Application("Super Mario Bros", WIN_W, WIN_H)
{}

void MarioGame::onInit() {
    m_map = Engine::Tilemap::loadMap("games/mario/assets/maps/1_1map.tmx");

    const auto* ts = m_map.tilesetForGid(1);
    m_tilesetTex   = std::make_shared<Engine::Texture>(ts->imagePath);
    m_tilesetSheet = std::make_shared<Engine::SpriteSheet>(m_tilesetTex, kSheetCols, kSheetRows);

    m_marioX = 3.f * TILE * SCALE;
}

void MarioGame::onUpdate(float dt) {
    if (Engine::Input::isKeyPressed(GLFW_KEY_Q))
        quit();

    static constexpr float kSpeed = 200.f;

    if (Engine::Input::isKeyDown(GLFW_KEY_RIGHT)) m_marioX += kSpeed * dt;
    if (Engine::Input::isKeyDown(GLFW_KEY_LEFT))  m_marioX -= kSpeed * dt;

    float mapWidth = static_cast<float>(m_map.cols * TILE * SCALE);
    m_marioX  = std::clamp(m_marioX, 0.f, mapWidth);
    m_cameraX = std::clamp(m_marioX - WIN_W * 0.5f, 0.f, mapWidth - WIN_W);
}

void MarioGame::onRender() {
    m_renderer.beginScene(WIN_W, WIN_H);
    renderBackground();
    renderTerrain();
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
