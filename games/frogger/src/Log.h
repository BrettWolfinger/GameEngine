#pragma once
#include "Platform.h"

class Log : public Platform {
public:
    Log(float startX, int row, int tileWidth, float speed, int direction);

    void render(Engine::Renderer2D&, const Engine::SpriteSheet&) const override;

private:
    static constexpr int FRAME_FIRST = 24;  // row 3 col 0
    static constexpr int FRAME_MID   = 25;  // row 3 col 1
    static constexpr int FRAME_LAST  = 26;  // row 3 col 2
};
