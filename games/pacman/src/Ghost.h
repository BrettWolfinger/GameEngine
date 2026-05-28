#pragma once
#include <engine/renderer/Renderer2D.h>
#include <engine/renderer/SpriteSheet.h>
#include <engine/renderer/SpriteAnimator.h>
#include <engine/tilemap/Tilemap.h>
#include "GameTypes.h"
#include "GameConstants.h"
#include <memory>
#include <vector>

class Ghost {
public:
    /// @param wallLayer  Cached Wall layer used for collision checks.
    /// @param doorLayer  Nullable pointer to the Door layer; used to detect passable door tiles.
    /// @param sheet      Shared ghost spritesheet (4x11, 32px native cells).
    /// @param faceSheet  Shared face spritesheet (4x1, 16px native cells).
    /// @param type       Ghost identity — determines scatter corner and chase target.
    /// @param startCol   Spawn column.
    /// @param startRow   Spawn row.
    /// @param homeMode   Mode used when first spawned and after respawn (Scatter or House).
    Ghost(const Engine::Tilemap::TileLayer& wallLayer,
          const Engine::Tilemap::TileLayer* doorLayer,
          std::shared_ptr<Engine::SpriteSheet> sheet,
          std::shared_ptr<Engine::SpriteSheet> faceSheet,
          GhostType type, int startCol, int startRow,
          GhostMode homeMode = GhostMode::Scatter);

    /// @param pacCol/pacRow  Pac-Man's current tile.
    /// @param pacDir        Pac-Man's current direction (used by Pinky and Inky).
    /// @param blinkyCol/Row Blinky's current tile (used by Inky).
    void update(float dt, int pacCol, int pacRow, Dir pacDir, int blinkyCol, int blinkyRow,
                float normalSpeed, float frightenedSpeed, float eyesSpeed);
    void render(Engine::Renderer2D& renderer, int renderLayer, float offsetY = 0.f) const;

    /// Switch scatter/chase mode and immediately reverse direction.
    void setMode(GhostMode mode);

    /// Enter frightened mode: reverse direction and switch to frightened sprite.
    /// No-op if in Eyes, House, or Leaving mode.
    void frighten();

    /// Switch to the flashing sprite to warn that frightened mode is ending.
    /// No-op if not currently frightened or already flashing.
    void startFlash();

    /// Exit frightened mode and return to returnMode without reversing direction.
    /// No-op if in Eyes, House, or Leaving mode.
    void endFrightened(GhostMode returnMode);

    /// Enter Eyes mode: body disappears and the face navigates back to spawn.
    void startEyes();

    /// Transition from House mode to Leaving mode, navigating up through the door.
    /// @param returnMode  The mode to enter once the ghost clears the door row.
    void release(GhostMode returnMode);

    /// Reset to spawn position and restart in home mode.
    void respawn();

    int       col()  const { return m_col; }
    int       row()  const { return m_row; }
    GhostType type() const { return m_type; }
    GhostMode mode() const { return m_mode; }

private:
    /// Returns true if the given tile is a wall.
    /// @param ignoreDoor  When true, door tiles are treated as passable.
    bool isWall(int col, int row, bool ignoreDoor = false) const;

    /// Returns true if the given tile belongs to the Door layer.
    bool isDoorTile(int col, int row) const;

    /// Returns the scatter-corner target tile for this ghost type.
    std::pair<int,int> scatterCorner() const;

    /// Returns the chase target tile for this ghost type based on cached Pac-Man/Blinky state.
    std::pair<int,int> chaseTarget() const;

    /// Returns the active target tile for the current mode.
    std::pair<int,int> targetTile() const;

    /// Picks the best next direction at the current intersection.
    /// Eyes mode uses precomputed BFS distances for true shortest-path navigation.
    /// All other modes use the classic Manhattan-distance heuristic.
    /// Tie-breaks by classic priority: Up > Left > Down > Right.
    /// Reverse direction is excluded except as a last-resort fallback at dead ends.
    Dir chooseDirection() const;

    void setTarget(Dir dir);

    /// Immediately swaps current and target cells to reverse direction.
    void reverseDirection();

    /// Returns movement speed in tiles/sec for the current mode.
    float currentSpeed(float normalSpeed, float frightenedSpeed, float eyesSpeed) const;

    const Engine::Tilemap::TileLayer&    m_wallLayer;
    const Engine::Tilemap::TileLayer*    m_doorLayer = nullptr;
    Engine::SpriteAnimator               m_animator;
    std::shared_ptr<Engine::SpriteSheet> m_faceSheet;

    GhostType m_type;
    GhostMode m_mode        = GhostMode::Scatter;
    GhostMode m_homeMode    = GhostMode::Scatter;
    GhostMode m_releaseMode = GhostMode::Scatter;
    float     m_x           = 0.f;
    float     m_y           = 0.f;
    int       m_col         = 0;
    int       m_row         = 0;
    int       m_tgtCol      = 0;
    int       m_tgtRow      = 0;
    Dir       m_dir         = Dir::None;
    bool      m_flashing    = false;

    // Spawn position — used by respawn() and Eyes mode targeting.
    int       m_startCol    = 0;
    int       m_startRow    = 0;

    // BFS distances from spawn, precomputed in constructor.
    // m_eyesDist[row * cols + col] = shortest tile steps to spawn; -1 if unreachable.
    // Used by chooseDirection() in Eyes mode for true shortest-path navigation.
    std::vector<int> m_eyesDist;

    // Cached per-frame context set by update() and read by chaseTarget().
    int       m_pacCol    = 0;
    int       m_pacRow    = 0;
    Dir       m_pacDir    = Dir::None;
    int       m_blinkyCol = 0;
    int       m_blinkyRow = 0;
};
