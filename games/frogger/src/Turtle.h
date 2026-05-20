#pragma once
#include "Platform.h"

class Turtle : public Platform {
public:
    Turtle(float startX, int row, int tileWidth, float speed, int direction);

    void render(Engine::Renderer2D&, const Engine::SpriteSheet&) const override;

private:
    static constexpr int FRAME = 17;  // row 2 col 1
};
