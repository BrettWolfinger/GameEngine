#include "Ghost.h"
#include <cmath>
#include <climits>
#include <cstdlib>

static constexpr float kGridSize    = static_cast<float>(TILE * SCALE);
static constexpr float kRenderSize  = static_cast<float>(TILE * 2 * SCALE);

static std::pair<int,int> dirOffset(Dir d) {
    switch (d) {
        case Dir::Up:    return { 0, -1};
        case Dir::Down:  return { 0,  1};
        case Dir::Left:  return {-1,  0};
        case Dir::Right: return { 1,  0};
        default:         return { 0,  0};
    }
}

static Dir opposite(Dir d) {
    switch (d) {
        case Dir::Up:    return Dir::Down;
        case Dir::Down:  return Dir::Up;
        case Dir::Left:  return Dir::Right;
        case Dir::Right: return Dir::Left;
        default:         return Dir::None;
    }
}

Ghost::Ghost(const Engine::Tilemap::TileLayer& wallLayer,
             std::shared_ptr<Engine::SpriteSheet> sheet,
             GhostType type, int startCol, int startRow)
    : m_wallLayer(wallLayer)
    , m_animator(sheet)
    , m_type(type)
    , m_col(startCol)
    , m_row(startRow)
    , m_startCol(startCol)
    , m_startRow(startRow)
{
    // Each ghost type occupies one row; columns are animation frames.
    const int base  = static_cast<int>(m_type) * kGhostSheetCols;
    const int fBase = kGhostFrightenedRow * kGhostSheetCols;
    const int wBase = kGhostFlashRow      * kGhostSheetCols;
    m_animator.addClip("move",       { {base,  base+1,  base+2,  base+3},  0.15f, Engine::PlayMode::Loop });
    m_animator.addClip("frightened", { {fBase, fBase+1, fBase+2, fBase+3}, 0.2f,  Engine::PlayMode::Loop });
    // Flash clip interleaves blue and white frames for the alternating warning effect.
    m_animator.addClip("frightened_flash", {
        {fBase, wBase, fBase+1, wBase+1, fBase+2, wBase+2, fBase+3, wBase+3}, 0.1f, Engine::PlayMode::Loop
    });
    m_animator.setClip("move");

    m_x = m_col * kGridSize + kGridSize * 0.5f;
    m_y = m_row * kGridSize + kGridSize * 0.5f;

    // Pick an initial direction toward the scatter corner and start moving.
    m_dir    = chooseDirection();
    m_tgtCol = m_col;
    m_tgtRow = m_row;
    setTarget(m_dir);
}

bool Ghost::isWall(int col, int row) const {
    if (row < 0 || row >= m_wallLayer.rows) return true;
    col = (col + m_wallLayer.cols) % m_wallLayer.cols;
    return Engine::Tilemap::stripFlips(m_wallLayer.gids[row * m_wallLayer.cols + col]) != 0;
}

std::pair<int,int> Ghost::scatterCorner() const {
    switch (m_type) {
        case GhostType::Blinky: return { MAP_COLS - 3, 0            }; // top-right
        case GhostType::Pinky:  return { 2,            0            }; // top-left
        case GhostType::Inky:   return { MAP_COLS - 3, MAP_ROWS - 1 }; // bottom-right
        case GhostType::Clyde:  return { 2,            MAP_ROWS - 1 }; // bottom-left
    }
    return { 0, 0 };
}

std::pair<int,int> Ghost::chaseTarget() const {
    auto [dc, dr] = dirOffset(m_pacDir);

    switch (m_type) {
        case GhostType::Blinky:
            // Target Pac-Man's current tile directly.
            return { m_pacCol, m_pacRow };

        case GhostType::Pinky:
            // Target 4 tiles ahead of Pac-Man.
            // Recreates the classic Up bug: moving up also offsets 4 tiles left.
            if (m_pacDir == Dir::Up)
                return { m_pacCol - 4, m_pacRow - 4 };
            return { m_pacCol + dc * 4, m_pacRow + dr * 4 };

        case GhostType::Inky: {
            // Take 2 tiles ahead of Pac-Man, then double the vector from Blinky to that point.
            int aheadCol = m_pacCol + dc * 2;
            int aheadRow = m_pacRow + dr * 2;
            return { aheadCol + (aheadCol - m_blinkyCol),
                     aheadRow + (aheadRow - m_blinkyRow) };
        }

        case GhostType::Clyde: {
            // Target Pac-Man when far (> 8 tiles), scatter corner when close.
            int dist = std::abs(m_col - m_pacCol) + std::abs(m_row - m_pacRow);
            return (dist > 8) ? std::make_pair(m_pacCol, m_pacRow) : scatterCorner();
        }
    }
    return { m_pacCol, m_pacRow };
}

std::pair<int,int> Ghost::targetTile() const {
    return (m_mode == GhostMode::Chase) ? chaseTarget() : scatterCorner();
}

Dir Ghost::chooseDirection() const {
    static constexpr Dir kAllDirs[] = { Dir::Up, Dir::Left, Dir::Down, Dir::Right };

    // Frightened: pick randomly from valid non-reverse directions.
    if (m_mode == GhostMode::Frightened) {
        const Dir rev = opposite(m_dir);
        Dir valid[4];
        int count = 0;
        for (Dir d : kAllDirs) {
            if (d == rev) continue;
            auto [dc, dr] = dirOffset(d);
            int nc = (m_col + dc + m_wallLayer.cols) % m_wallLayer.cols;
            int nr = m_row + dr;
            if (!isWall(nc, nr))
                valid[count++] = d;
        }
        if (count > 0) return valid[std::rand() % count];
        return rev;
    }

    const Dir rev = opposite(m_dir); // direction ghosts may not reverse into
    auto [tCol, tRow] = targetTile();

    int bestDist = INT_MAX;
    Dir bestDir  = Dir::None;

    for (Dir d : kAllDirs) {
        if (d == rev) continue;
        auto [dc, dr] = dirOffset(d);
        int nc = (m_col + dc + m_wallLayer.cols) % m_wallLayer.cols;
        int nr = m_row + dr;
        if (isWall(nc, nr)) continue;

        int dist = std::abs(nc - tCol) + std::abs(nr - tRow);
        if (dist < bestDist) {
            bestDist = dist;
            bestDir  = d;
        }
    }

    // Dead end — reverse (only happens in narrow corridors, not in the Pac-Man maze).
    if (bestDir == Dir::None)
        bestDir = rev;

    return bestDir;
}

void Ghost::reverseDirection() {
    // Guard: if already at cell center (e.g. first frame), nothing to reverse.
    if (m_col == m_tgtCol && m_row == m_tgtRow) return;
    std::swap(m_col, m_tgtCol);
    std::swap(m_row, m_tgtRow);
    m_dir = opposite(m_dir);
}

void Ghost::setMode(GhostMode mode) {
    if (m_mode == mode) return;
    m_mode = mode;
    reverseDirection();
}

void Ghost::frighten() {
    if (m_mode == GhostMode::Eyes) return; // eaten ghosts are unaffected
    m_mode     = GhostMode::Frightened;
    m_flashing = false;
    reverseDirection();
    m_animator.setClip("frightened");
}

void Ghost::startFlash() {
    if (m_mode != GhostMode::Frightened || m_flashing) return;
    m_flashing = true;
    m_animator.setClip("frightened_flash");
}

void Ghost::endFrightened(GhostMode returnMode) {
    m_mode     = returnMode;
    m_flashing = false;
    m_animator.setClip("move");
}

void Ghost::respawn() {
    m_col    = m_startCol;
    m_row    = m_startRow;
    m_tgtCol = m_startCol;
    m_tgtRow = m_startRow;
    m_x      = m_startCol * kGridSize + kGridSize * 0.5f;
    m_y      = m_startRow * kGridSize + kGridSize * 0.5f;
    m_mode   = GhostMode::Scatter;
    m_dir    = chooseDirection();
    setTarget(m_dir);
    m_animator.setClip("move");
}

float Ghost::currentSpeed(float normalSpeed, float frightenedSpeed) const {
    return (m_mode == GhostMode::Frightened) ? frightenedSpeed : normalSpeed;
}

void Ghost::setTarget(Dir dir) {
    auto [dc, dr] = dirOffset(dir);
    m_tgtCol = (m_col + dc + m_wallLayer.cols) % m_wallLayer.cols;
    m_tgtRow = m_row + dr;
}

void Ghost::update(float dt, int pacCol, int pacRow, Dir pacDir, int blinkyCol, int blinkyRow,
                   float normalSpeed, float frightenedSpeed) {
    m_pacCol    = pacCol;
    m_pacRow    = pacRow;
    m_pacDir    = pacDir;
    m_blinkyCol = blinkyCol;
    m_blinkyRow = blinkyRow;
    // Adjust target x for tunnel wrap so the sprite exits one side and enters the other.
    float tx;
    if      (m_dir == Dir::Left  && m_tgtCol > m_col)
        tx = (m_tgtCol - m_wallLayer.cols) * kGridSize + kGridSize * 0.5f;
    else if (m_dir == Dir::Right && m_tgtCol < m_col)
        tx = (m_tgtCol + m_wallLayer.cols) * kGridSize + kGridSize * 0.5f;
    else
        tx = m_tgtCol * kGridSize + kGridSize * 0.5f;
    float ty   = m_tgtRow * kGridSize + kGridSize * 0.5f;
    float dx   = tx - m_x;
    float dy   = ty - m_y;
    float dist = std::abs(dx) + std::abs(dy);
    float step = currentSpeed(normalSpeed, frightenedSpeed) * kGridSize * dt;

    if (step >= dist) {
        m_col = m_tgtCol;
        m_row = m_tgtRow;
        m_x   = m_col * kGridSize + kGridSize * 0.5f; // snap to actual on-screen position
        m_y   = m_row * kGridSize + kGridSize * 0.5f;

        m_dir = chooseDirection();
        setTarget(m_dir);
    } else {
        m_x += (dx / dist) * step;
        m_y += (dy / dist) * step;
    }

    m_animator.update(dt);
}

void Ghost::render(Engine::Renderer2D& renderer, int renderLayer, float offsetY) const {
    const auto uv = m_animator.currentFrameUVs();

    renderer.drawTexturedRect(
        m_x - kRenderSize * 0.5f, m_y - kRenderSize * 0.5f + offsetY,
        kRenderSize, kRenderSize,
        m_animator.sheet().texture(),
        uv.u0, uv.v0, uv.u1, uv.v1,
        0.f, {1.f, 1.f, 1.f, 1.f},
        renderLayer
    );
}
