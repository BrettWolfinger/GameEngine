#include "FroggerGame.h"
#include <engine/renderer/Texture.h>
#include <engine/renderer/SpriteSheet.h>
#include <engine/core/Input.h>
#include <GLFW/glfw3.h>
#include <algorithm>

static constexpr int COLS     = 13;
static constexpr int ROWS     = 14;
static constexpr int TILE_SRC = 16;        // sprite sheet cell size (px)
static constexpr int SCALE    = 3;
static constexpr int TILE     = TILE_SRC * SCALE; // 48px on screen
static constexpr int W        = COLS * TILE;      // 624
static constexpr int H        = ROWS * TILE;      // 672

// Sprite sheet layout: 8 cols x 16 rows of 16x16px cells
static constexpr int SHEET_COLS = 8;
static constexpr int SHEET_ROWS = 16;

// Hop animation: 4 frames x 0.1s each
static constexpr int   HOP_FRAMES   = 4;
static constexpr float HOP_FRAME_DT = 0.1f;
static constexpr float HOP_DURATION = HOP_FRAMES * HOP_FRAME_DT;

FroggerGame::FroggerGame()
    : Engine::Application("Frogger", W, H)
{
    auto texture = std::make_shared<Engine::Texture>("games/frogger/assets/frogger_sprite_sheet.png");
    m_sheet      = std::make_shared<Engine::SpriteSheet>(texture, SHEET_COLS, SHEET_ROWS);

    m_animator.emplace(m_sheet);

    Engine::AnimClip idleClip;
    idleClip.frames        = { 0 };
    idleClip.frameDuration = 1.f;
    idleClip.mode          = Engine::PlayMode::Loop;

    Engine::AnimClip hopClip;
    hopClip.frames        = { 0, 1, 2, 3 };
    hopClip.frameDuration = HOP_FRAME_DT;
    hopClip.mode          = Engine::PlayMode::Loop;

    m_animator->addClip("idle", std::move(idleClip));
    m_animator->addClip("hop",  std::move(hopClip));
    m_animator->setClip("idle");
}

void FroggerGame::onUpdate(float dt) {
    if (Engine::Input::isKeyPressed(GLFW_KEY_Q))
        quit();

    // Finish hop before accepting new input
    if (m_frog.hopping) {
        m_frog.hopTimer -= dt;
        if (m_frog.hopTimer <= 0.f) {
            m_frog.hopping = false;
            m_animator->setClip("idle");
        }
    } else {
        int dc = 0, dr = 0;
        if (Engine::Input::isKeyPressed(GLFW_KEY_UP)    || Engine::Input::isKeyPressed(GLFW_KEY_W)) dr = -1;
        if (Engine::Input::isKeyPressed(GLFW_KEY_DOWN)  || Engine::Input::isKeyPressed(GLFW_KEY_S)) dr =  1;
        if (Engine::Input::isKeyPressed(GLFW_KEY_LEFT)  || Engine::Input::isKeyPressed(GLFW_KEY_A)) dc = -1;
        if (Engine::Input::isKeyPressed(GLFW_KEY_RIGHT) || Engine::Input::isKeyPressed(GLFW_KEY_D)) dc =  1;

        if (dc != 0 || dr != 0) {
            m_frog.col = std::clamp(m_frog.col + dc, 0, COLS - 1);
            m_frog.row = std::clamp(m_frog.row + dr, 0, ROWS - 1);
            m_frog.hopping  = true;
            m_frog.hopTimer = HOP_DURATION;
            m_animator->setClip("hop");
        }
    }

    m_animator->update(dt);
}

void FroggerGame::onRender() {
    m_renderer.beginScene(W, H);

    const float x = static_cast<float>(m_frog.col * TILE);
    const float y = static_cast<float>(m_frog.row * TILE);

    const Engine::UVRect uvs = m_animator->currentFrameUVs();
    m_renderer.drawTexturedRect(x, y, TILE, TILE,
                                m_sheet->texture(),
                                uvs.u0, uvs.v0, uvs.u1, uvs.v1);
}
