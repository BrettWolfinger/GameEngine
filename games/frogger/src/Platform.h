#pragma once
#include "LaneObject.h"

class Platform : public LaneObject {
public:
    Platform(float startX, int row, PlatformType type, int tileWidth, float speed, int direction);

    void render(Engine::Renderer2D& renderer, const Engine::SpriteSheet& sheet) const override;

    virtual bool isSafe() const { return true; }

private:
    PlatformType m_type;
};
