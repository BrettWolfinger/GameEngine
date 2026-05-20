#pragma once
#include "LaneObject.h"

class Vehicle : public LaneObject {
public:
    Vehicle(float startX, int row, VehicleType type, float speed, int direction);

    void render(Engine::Renderer2D& renderer, const Engine::SpriteSheet& sheet) const override;

private:
    VehicleType m_type;
};
