#pragma once
#include <engine/renderer/Renderer2D.h>
#include <engine/renderer/SpriteSheet.h>
#include "FroggerConfig.h"

class Platform {
public:
    Platform(float startX, int row, PlatformType type, float speed, int direction);

    void update(float dt);
    void render(Engine::Renderer2D& renderer, const Engine::SpriteSheet& sheet) const;

    float x()         const { return m_x; }
    int   row()       const { return m_row; }
    int   tileWidth() const;
    float velocityX() const { return m_speed * static_cast<float>(m_direction); }

private:
    float        m_x;
    int          m_row;
    PlatformType m_type;
    float        m_speed;
    int          m_direction;
};
