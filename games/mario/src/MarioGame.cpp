#include "MarioGame.h"
#include "GameConstants.h"
#include <GLFW/glfw3.h>
#include <algorithm> // std::clamp

static constexpr int kLayerBackground = 0;
static constexpr int kLayerTerrain    = 2;
static constexpr int kLayerEnemy      = 3;
static constexpr int kLayerPlayer     = 4;



MarioGame::MarioGame()
    : Engine::Application("Super Mario Bros", WIN_W, WIN_H)
{}

void MarioGame::onInit() {
    m_map = Engine::Tilemap::loadMap("games/mario/assets/maps/1_1map.tmx");

    const auto* ts = m_map.tilesetForGid(1);
    m_tilesetTex   = std::make_shared<Engine::Texture>(ts->imagePath);
    m_tilesetSheet = std::make_shared<Engine::SpriteSheet>(m_tilesetTex, Engine::FrameSize{ TILE, TILE });

    m_marioTex   = std::make_shared<Engine::Texture>("games/mario/assets/sprites/smb-mario.png");
    m_marioSheet = std::make_shared<Engine::SpriteSheet>(m_marioTex, Engine::FrameSize{ MARIO_FRAME_W, MARIO_FRAME_H });

    m_enemiesTex   = std::make_shared<Engine::Texture>("games/mario/assets/sprites/smb-enemies.png");
    m_enemiesSheet = std::make_shared<Engine::SpriteSheet>(m_enemiesTex, Engine::FrameSize{ GOOMBA_FRAME_W, GOOMBA_FRAME_H });

    m_startX = 3.f * TILE * SCALE;
    m_startY = static_cast<float>((SCREEN_ROWS - 2) * TILE * SCALE - MARIO_FRAME_H * SCALE);
    registerConfig("games/mario/assets/configs/mario.toml",  &m_config);
    registerConfig("games/mario/assets/configs/goomba.toml", &m_goombaConfig);

    const auto* terrain = m_map.findLayer("Terrain");
    if (terrain)
        m_collider.emplace(*terrain, TILE * SCALE, TILE * SCALE,
                           [](uint32_t gid) { return gid != 0; });

    m_player = std::make_unique<Player>(m_marioSheet, m_startX, m_startY);

    // Spawn a couple of Goombas at fixed tile positions
    const float goombaY = static_cast<float>((SCREEN_ROWS - 2) * TILE * SCALE - GOOMBA_FRAME_H * SCALE);
    m_goombas.emplace_back(m_enemiesSheet, 22.f * TILE * SCALE, goombaY);
    m_goombas.emplace_back(m_enemiesSheet, 23.f * TILE * SCALE, goombaY);
}

void MarioGame::onUpdate(float dt) {
    if (Engine::Input::isKeyPressed(GLFW_KEY_Q))
        quit();

    if (m_collider) {
        m_player->update(dt, m_config, *m_collider);
        for (auto& g : m_goombas)
            g.update(dt, m_config.gravity, m_goombaConfig, *m_collider);
        checkEnemyCollisions();
    }

    if (m_player->isDead())
        m_player->respawn(m_startX, m_startY);

    const float playerCenterX = m_player->x() + MARIO_FRAME_W * 0.5f * SCALE;
    const float mapWidth      = static_cast<float>(m_map.cols * TILE * SCALE);
    m_cameraX = std::clamp(playerCenterX - WIN_W * 0.5f, 0.f, mapWidth - WIN_W);
}

void MarioGame::onRender() {
    m_renderer.beginScene(WIN_W, WIN_H);
    renderBackground();
    renderTerrain();
    renderEnemies();
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

void MarioGame::renderEnemies() {
    for (const auto& g : m_goombas)
        g.render(m_renderer, m_cameraX, kLayerEnemy);
}

void MarioGame::checkEnemyCollisions() {
    const float mx = m_player->hitboxX();
    const float my = m_player->hitboxY();
    const float mw = m_player->hitboxW();
    const float mh = m_player->hitboxH();

    for (auto& g : m_goombas) {
        if (g.isDead()) continue;

        // AABB overlap
        if (mx + mw <= g.hitboxX() || mx >= g.hitboxX() + g.hitboxW()) continue;
        if (my + mh <= g.hitboxY() || my >= g.hitboxY() + g.hitboxH()) continue;

        // Stomp: Mario falling and his hitbox bottom is above the Goomba's center
        if (m_player->vy() > 0.f && my + mh < g.hitboxY() + g.hitboxH() * 0.5f)
            g.stomp(), m_player->onStompGoomba(m_config);
        else
            m_player->onHitByEnemy();
    }
}
