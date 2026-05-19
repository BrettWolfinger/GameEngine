#include "FroggerGame.h"
#include <engine/renderer/Texture.h>
#include <engine/renderer/SpriteSheet.h>

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

FroggerGame::FroggerGame()
    : Engine::Application("Frogger", W, H)
{
    auto texture = std::make_shared<Engine::Texture>("games/frogger/assets/frogger_sprite_sheet.png");
    m_sheet      = std::make_shared<Engine::SpriteSheet>(texture, SHEET_COLS, SHEET_ROWS);

    m_animator.emplace(m_sheet);

    // Frog hop animation — first 4 frames of the sheet (top row)
    Engine::AnimClip hopClip;
    hopClip.frames        = { 0, 1, 2, 3 };
    hopClip.frameDuration = 0.1f;
    hopClip.mode          = Engine::PlayMode::Loop;

    m_animator->addClip("hop", std::move(hopClip));
    m_animator->setClip("hop");
}

void FroggerGame::onUpdate(float dt) {
    m_animator->update(dt);
}

void FroggerGame::onRender() {
    m_renderer.beginScene(W, H);

    // Draw the frog centered in the window at 1 tile (48x48px)
    const float x = (W - TILE) * 0.5f;
    const float y = (H - TILE) * 0.5f;

    const Engine::UVRect uvs = m_animator->currentFrameUVs();
    m_renderer.drawTexturedRect(x, y, TILE, TILE,
                                m_sheet->texture(),
                                uvs.u0, uvs.v0, uvs.u1, uvs.v1);
}
