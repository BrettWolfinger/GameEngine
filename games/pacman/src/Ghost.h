#pragma once
#include <engine/renderer/Renderer2D.h>
#include <engine/renderer/SpriteSheet.h>
#include <engine/renderer/SpriteAnimator.h>
#include <engine/tilemap/Tilemap.h>
#include "GameTypes.h"
#include "GameConstants.h"
#include <memory>

class Ghost {
public:
    /// @param wallLayer  Cached Wall layer used for collision checks.
    /// @param sheet      Shared ghost spritesheet (8x22, 16px native cells).
    /// @param type       Ghost identity — determines scatter corner and later chase target.
    /// @param startCol   Spawn column.
    /// @param startRow   Spawn row.
    Ghost(const Engine::Tilemap::TileLayer& wallLayer,
          std::shared_ptr<Engine::SpriteSheet> sheet,
          GhostType type, int startCol, int startRow);

    /// @param pacCol/pacRow  Pac-Man's current tile.
    /// @param pacDir        Pac-Man's current direction (used by Pinky and Inky).
    /// @param blinkyCol/Row Blinky's current tile (used by Inky).
    void update(float dt, int pacCol, int pacRow, Dir pacDir, int blinkyCol, int blinkyRow);
    void render(Engine::Renderer2D& renderer, int renderLayer) const;

    /// Switch scatter/chase mode and immediately reverse direction.
    void setMode(GhostMode mode);

    /// Enter frightened mode: reverse direction and switch to frightened sprite.
    /// No-op if already in Eyes mode (eaten ghosts are unaffected).
    void frighten();

    /// Exit frightened mode and return to returnMode without reversing direction.
    void endFrightened(GhostMode returnMode);

    /// Reset to spawn position and restart in Scatter mode.
    void respawn();

    int       col()  const { return m_col; }
    int       row()  const { return m_row; }
    GhostType type() const { return m_type; }
    GhostMode mode() const { return m_mode; }

private:
    bool isWall(int col, int row) const;

    /// Returns the scatter-corner target tile for this ghost type.
    std::pair<int,int> scatterCorner() const;

    /// Returns the chase target tile for this ghost type based on cached Pac-Man/Blinky state.
    std::pair<int,int> chaseTarget() const;

    /// Returns the active target tile (scatter corner or chase target) for the current mode.
    std::pair<int,int> targetTile() const;

    /// Picks the best next direction at the current intersection.
    /// Excludes the reverse of the current direction (no U-turns).
    /// Tie-breaks by classic priority: Up > Left > Down > Right.
    Dir chooseDirection() const;

    void setTarget(Dir dir);

    /// Immediately swaps current and target cells to reverse direction.
    void reverseDirection();

    /// Returns movement speed in tiles/sec for the current mode.
    float currentSpeed() const;

    const Engine::Tilemap::TileLayer& m_wallLayer;
    Engine::SpriteAnimator            m_animator;

    GhostType m_type;
    GhostMode m_mode      = GhostMode::Scatter;
    float     m_x         = 0.f;
    float     m_y         = 0.f;
    int       m_col       = 0;
    int       m_row       = 0;
    int       m_tgtCol    = 0;
    int       m_tgtRow    = 0;
    Dir       m_dir       = Dir::None;

    // Spawn position — used by respawn().
    int       m_startCol  = 0;
    int       m_startRow  = 0;

    // Cached per-frame context set by update() and read by chaseTarget().
    int       m_pacCol    = 0;
    int       m_pacRow    = 0;
    Dir       m_pacDir    = Dir::None;
    int       m_blinkyCol = 0;
    int       m_blinkyRow = 0;
};
