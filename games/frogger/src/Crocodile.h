#pragma once
#include "Platform.h"

class Crocodile : public Platform {
public:
    Crocodile(float startX, int row, float speed, int direction);

    void render(Engine::Renderer2D&, const Engine::SpriteSheet&) const override;

protected:
    void onUpdate(float dt) override;

private:
    static constexpr int TILE_WIDTH = 3;

    static constexpr int TAIL_FRAMES[] = { 32, 40, 41, 42, 43 }; // row 4 col 0, row 5 col 0-3
    static constexpr int BODY_FRAMES[] = { 33, 44, 45, 46, 47 }; // row 4 col 1, row 5 col 4-7
    static constexpr int HEAD_FRAMES[] = { 34, 35, 36, 37, 38 }; // row 4 col 2-6

    static constexpr int   FRAME_COUNT    = 5;
    static constexpr float FRAME_DURATION = 0.15f;

    float m_animTimer = 0.f;
    int   m_frameIdx  = 0;
};
