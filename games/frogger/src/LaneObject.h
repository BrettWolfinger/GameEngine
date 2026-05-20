#pragma once
#include <engine/renderer/Renderer2D.h>
#include <engine/renderer/SpriteSheet.h>
#include "FroggerConfig.h"

class LaneObject {
public:
    LaneObject(float startX, int row, int tileWidth, float speed, int direction);
    virtual ~LaneObject() = default;

    // Non-virtual: always runs movement+wrapping, then calls onUpdate
    void update(float dt);

    virtual void render(Engine::Renderer2D& renderer, const Engine::SpriteSheet& sheet) const = 0;

    float x()         const { return m_x; }
    int   row()       const { return m_row; }
    int   tileWidth() const { return m_tileWidth; }
    float velocityX() const { return m_speed * static_cast<float>(m_direction); }

protected:
    float m_x;
    int   m_row;
    int   m_tileWidth;
    float m_speed;
    int   m_direction;

    virtual void onUpdate(float dt) {}
};
