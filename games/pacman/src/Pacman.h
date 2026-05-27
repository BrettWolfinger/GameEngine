#pragma once
#include <engine/renderer/Renderer2D.h>
#include <engine/renderer/SpriteSheet.h>
#include <engine/renderer/SpriteAnimator.h>
#include <engine/tilemap/Tilemap.h>
#include "GameTypes.h"
#include "GameConstants.h"
#include <memory>

// Spritesheet layout: 4 cols x 3 rows, 16px native cells rendered at TILE * SCALE
static constexpr int kPacFrameSize = TILE * 2;
static constexpr int kPacSheetCols = 4;
static constexpr int kPacSheetRows = 3;

class Pacman {
public:
    /// @param wallLayer  Cached Wall layer used for collision checks.
    /// @param sheet      Pac-Man spritesheet (4x3, 32x32 cells).
    /// @param startCol   Spawn column (from object layer or fallback default).
    /// @param startRow   Spawn row (from object layer or fallback default).
    Pacman(const Engine::Tilemap::TileLayer& wallLayer,
           std::shared_ptr<Engine::SpriteSheet> sheet,
           int startCol, int startRow);

    void update(float dt);
    void render(Engine::Renderer2D& renderer, int renderLayer) const;

    /// Begin the death animation; freezes movement and input.
    void startDeath();

    /// Returns true once the OneShot death animation has played through.
    bool isDeathDone() const;

    /// Reset to spawn position and resume normal movement/animation.
    void respawn();

    int col()     const { return m_col; }
    int row()     const { return m_row; }
    Dir dir()     const { return m_dir; }
    bool isDying() const { return m_dying; }

private:
    bool isWall(int col, int row) const;
    bool canMove(int col, int row, Dir dir) const;
    void setTarget(int fromCol, int fromRow, Dir dir);

    const Engine::Tilemap::TileLayer& m_wallLayer;
    Engine::SpriteAnimator            m_animator;

    float m_x        = 0.f;
    float m_y        = 0.f;
    int   m_col      = 0;
    int   m_row      = 0;
    int   m_tgtCol   = 0;
    int   m_tgtRow   = 0;
    int   m_startCol = 0;
    int   m_startRow = 0;
    Dir   m_dir      = Dir::None;
    Dir   m_nextDir  = Dir::None;
    bool  m_dying    = false;
};
