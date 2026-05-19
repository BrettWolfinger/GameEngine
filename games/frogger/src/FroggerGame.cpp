#include "FroggerGame.h"

// stb_image_write — compiled once here (engine only compiles stb_image, not stb_image_write)
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <engine/renderer/Texture.h>
#include <engine/renderer/SpriteSheet.h>

#include <filesystem>
#include <vector>

static constexpr int FRAME_W    = 32;
static constexpr int FRAME_H    = 32;
static constexpr int FRAME_COUNT = 4;
static constexpr int SHEET_W    = FRAME_W * FRAME_COUNT; // 128
static constexpr int SHEET_H    = FRAME_H;               // 32

// ---------------------------------------------------------------------------
// Placeholder asset generation
// ---------------------------------------------------------------------------

void FroggerGame::ensurePlaceholderAsset(const std::string& path) {
    if (std::filesystem::exists(path)) return;

    // Create parent directory if needed
    std::filesystem::create_directories(std::filesystem::path(path).parent_path());

    // 4 frames: red, green, blue, yellow  (RGBA8)
    struct RGBA { unsigned char r, g, b, a; };
    const RGBA palette[FRAME_COUNT] = {
        { 220,  50,  50, 255 }, // red
        {  50, 200,  50, 255 }, // green
        {  50, 100, 220, 255 }, // blue
        { 220, 200,  50, 255 }, // yellow
    };

    std::vector<RGBA> pixels(static_cast<size_t>(SHEET_W) * SHEET_H);
    for (int f = 0; f < FRAME_COUNT; ++f) {
        for (int py = 0; py < FRAME_H; ++py) {
            for (int px = 0; px < FRAME_W; ++px) {
                const int idx = py * SHEET_W + f * FRAME_W + px;
                pixels[idx] = palette[f];
            }
        }
    }

    stbi_write_png(path.c_str(),
                   SHEET_W, SHEET_H,
                   4 /* channels */,
                   pixels.data(),
                   SHEET_W * static_cast<int>(sizeof(RGBA)));
}

// ---------------------------------------------------------------------------
// FroggerGame
// ---------------------------------------------------------------------------

FroggerGame::FroggerGame()
    : Engine::Application("Frogger", 800, 600)
{
    const std::string assetPath = "games/frogger/assets/frogger_test.png";
    ensurePlaceholderAsset(assetPath);

    auto texture = std::make_shared<Engine::Texture>(assetPath);
    m_sheet      = std::make_shared<Engine::SpriteSheet>(texture, FRAME_COUNT, 1);

    m_animator.emplace(m_sheet);

    Engine::AnimClip walkClip;
    for (int i = 0; i < m_sheet->frameCount(); ++i)
        walkClip.frames.push_back(i);
    walkClip.frameDuration = 0.2f; // 5 fps — clearly visible color changes
    walkClip.mode          = Engine::PlayMode::Loop;

    m_animator->addClip("walk", std::move(walkClip));
    m_animator->setClip("walk");
}

void FroggerGame::onUpdate(float dt) {
    m_animator->update(dt);
}

void FroggerGame::onRender() {
    auto& win = getWindow();
    const int w = win.getWidth();
    const int h = win.getHeight();

    m_renderer.beginScene(w, h);

    // Draw a 128x128 sprite centered in the window
    const float spriteW = 128.f;
    const float spriteH = 128.f;
    const float x = (w - spriteW) * 0.5f;
    const float y = (h - spriteH) * 0.5f;

    const Engine::UVRect uvs = m_animator->currentFrameUVs();
    m_renderer.drawTexturedRect(x, y, spriteW, spriteH,
                                m_sheet->texture(),
                                uvs.u0, uvs.v0, uvs.u1, uvs.v1);
}
