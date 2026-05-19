#pragma once
#include <engine/renderer/Renderer2D.h>
#include <engine/renderer/SpriteSheet.h>
#include "FroggerConfig.h"

class Vehicle {
public:
    Vehicle(float startX, int row, VehicleType type, float speed, int direction);

    void update(float dt);
    void render(Engine::Renderer2D& renderer, const Engine::SpriteSheet& sheet) const;

    float x()         const { return m_x; }
    int   row()       const { return m_row; }
    int   tileWidth() const;

private:
    float       m_x;
    int         m_row;
    VehicleType m_type;
    float       m_speed;
    int         m_direction;
};
