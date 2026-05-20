#pragma once
#include "Platform.h"

enum class MouthState { Closed, Opening, Open, Closing };

class Crocodile : public Platform {
public:
    Crocodile(float startX, int row, float speed, int direction, float mouthPhase = 0.f);

    void render(Engine::Renderer2D&, const Engine::SpriteSheet&) const override;
    bool isSafe(float frogPx) const override;

protected:
    void onUpdate(float dt) override;

private:
    static constexpr int TILE_WIDTH = 3;

    // Swim animation (all tiles)
    static constexpr int   TAIL_FRAMES[]     = { 32, 40, 41, 42, 43 }; // row 4 col 0, row 5 col 0-3
    static constexpr int   BODY_FRAMES[]     = { 33, 44, 45, 46, 47 }; // row 4 col 1, row 5 col 4-7
    static constexpr int   HEAD_FRAMES[]     = { 34, 35, 36, 37, 38 }; // row 4 col 2-6
    static constexpr int   FRAME_COUNT       = 5;
    static constexpr float FRAME_DURATION    = 0.15f;

    // Mouth animation (head tile only)
    static constexpr int   MOUTH_OPEN_FRAMES[] = { 69, 70, 71 };       // row 8 col 5-7
    static constexpr int   MOUTH_SWIM_FRAMES[] = { 12, 13, 14, 15 };   // row 1 col 4-7
    static constexpr int   MOUTH_OPEN_COUNT    = 3;
    static constexpr int   MOUTH_SWIM_COUNT    = 4;
    static constexpr float MOUTH_FRAME_DT      = 0.1f;
    static constexpr float CLOSED_DURATION     = 4.0f;
    static constexpr float OPEN_DURATION       = 3.0f;

    float      m_animTimer  = 0.f;
    int        m_frameIdx   = 0;
    MouthState m_mouthState = MouthState::Closed;
    float      m_mouthTimer = 0.f;
};
