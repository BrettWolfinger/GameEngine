#pragma once
#include <engine/renderer/Renderer2D.h>
#include <engine/tilemap/Tilemap.h>
#include "GameTypes.h"
#include "GameConstants.h"

class Pacman {
public:
    /// @param wallLayer  Cached Wall layer used for collision checks.
    explicit Pacman(const Engine::Tilemap::TileLayer& wallLayer);

    void update(float dt);
    void render(Engine::Renderer2D& renderer, int renderLayer) const;

    int col() const { return m_col; }
    int row() const { return m_row; }

private:
    bool isWall(int col, int row) const;
    bool canMove(int col, int row, Dir dir) const;
    void setTarget(int fromCol, int fromRow, Dir dir);

    const Engine::Tilemap::TileLayer& m_wallLayer;

    float m_x       = 0.f;        // world position (center, pixels)
    float m_y       = 0.f;
    int   m_col     = 14;         // current cell (snapped)
    int   m_row     = 23;
    int   m_tgtCol  = 14;         // cell currently moving toward
    int   m_tgtRow  = 23;
    Dir   m_dir     = Dir::None;  // active movement direction
    Dir   m_nextDir = Dir::None;  // buffered input direction
};
