#include "Ghost.h"
#include <cmath>
#include <climits>
#include <cstdlib>
#include <queue>

static constexpr Dir kAllDirs[] = { Dir::Up, Dir::Left, Dir::Down, Dir::Right };

static constexpr float kGridSize    = static_cast<float>(TILE * SCALE);
static constexpr float kRenderSize  = static_cast<float>(TILE * 2 * SCALE);
static constexpr float kFaceSize    = static_cast<float>(TILE * SCALE);      // 16px native × SCALE, centred on body

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
             std::shared_ptr<Engine::SpriteSheet> faceSheet,
             GhostType type, int startCol, int startRow)
    : m_wallLayer(wallLayer)
    , m_animator(sheet)
    , m_faceSheet(std::move(faceSheet))
    , m_type(type)
    , m_col(startCol)
    , m_row(startRow)
    , m_startCol(startCol)
    , m_startRow(startRow)
{
    const int base  = static_cast<int>(m_type) * kGhostSheetCols;
    const int fBase = kGhostFrightenedRow * kGhostSheetCols;
    const int wBase = kGhostFlashRow      * kGhostSheetCols;
    m_animator.addClip("move",       { {base,  base+1,  base+2,  base+3},  0.15f, Engine::PlayMode::Loop });
    m_animator.addClip("frightened", { {fBase, fBase+1, fBase+2, fBase+3}, 0.2f,  Engine::PlayMode::Loop });
    m_animator.addClip("frightened_flash", {
        {fBase, wBase, fBase+1, wBase+1, fBase+2, wBase+2, fBase+3, wBase+3}, 0.1f, Engine::PlayMode::Loop
    });
    m_animator.setClip("move");

    m_x = m_col * kGridSize + kGridSize * 0.5f;
    m_y = m_row * kGridSize + kGridSize * 0.5f;

    m_dir    = chooseDirection();
    m_tgtCol = m_col;
    m_tgtRow = m_row;
    setTarget(m_dir);

    // Precompute BFS distances from spawn for Eyes-mode shortest-path navigation.
    // A BFS flood-fill from the spawn tile assigns every reachable tile its true
    // shortest step-count, which chooseDirection() uses instead of Manhattan distance
    // when in Eyes mode.
    const int totalTiles = m_wallLayer.rows * m_wallLayer.cols;
    m_eyesDist.assign(totalTiles, -1);
    m_eyesDist[m_startRow * m_wallLayer.cols + m_startCol] = 0;
    std::queue<std::pair<int,int>> frontier;
    frontier.push({m_startCol, m_startRow});
    while (!frontier.empty()) {
        auto [c, r] = frontier.front();
        frontier.pop();
        for (Dir d : kAllDirs) {
            auto [dc, dr] = dirOffset(d);
            int nc = (c + dc + m_wallLayer.cols) % m_wallLayer.cols;
            int nr = r + dr;
            if (nr < 0 || nr >= m_wallLayer.rows) continue;
            if (isWall(nc, nr)) continue;
            const int idx = nr * m_wallLayer.cols + nc;
            if (m_eyesDist[idx] != -1) continue;
            m_eyesDist[idx] = m_eyesDist[r * m_wallLayer.cols + c] + 1;
            frontier.push({nc, nr});
        }
    }
}

bool Ghost::isWall(int col, int row) const {
    if (row < 0 || row >= m_wallLayer.rows) return true;
    col = (col + m_wallLayer.cols) % m_wallLayer.cols;
    return Engine::Tilemap::stripFlips(m_wallLayer.gids[row * m_wallLayer.cols + col]) != 0;
}

std::pair<int,int> Ghost::scatterCorner() const {
    switch (m_type) {
        case GhostType::Blinky: return { MAP_COLS - 3, 0            };
        case GhostType::Pinky:  return { 2,            0            };
        case GhostType::Inky:   return { MAP_COLS - 3, MAP_ROWS - 1 };
        case GhostType::Clyde:  return { 2,            MAP_ROWS - 1 };
    }
    return { 0, 0 };
}

std::pair<int,int> Ghost::chaseTarget() const {
    auto [dc, dr] = dirOffset(m_pacDir);

    switch (m_type) {
        case GhostType::Blinky:
            return { m_pacCol, m_pacRow };

        case GhostType::Pinky:
            if (m_pacDir == Dir::Up)
                return { m_pacCol - 4, m_pacRow - 4 };
            return { m_pacCol + dc * 4, m_pacRow + dr * 4 };

        case GhostType::Inky: {
            int aheadCol = m_pacCol + dc * 2;
            int aheadRow = m_pacRow + dr * 2;
            return { aheadCol + (aheadCol - m_blinkyCol),
                     aheadRow + (aheadRow - m_blinkyRow) };
        }

        case GhostType::Clyde: {
            int dist = std::abs(m_col - m_pacCol) + std::abs(m_row - m_pacRow);
            return (dist > 8) ? std::make_pair(m_pacCol, m_pacRow) : scatterCorner();
        }
    }
    return { m_pacCol, m_pacRow };
}

std::pair<int,int> Ghost::targetTile() const {
    switch (m_mode) {
        case GhostMode::Chase: return chaseTarget();
        case GhostMode::Eyes:  return { m_startCol, m_startRow };
        default:               return scatterCorner();
    }
}

Dir Ghost::chooseDirection() const {
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

    // Reverse direction is excluded in all non-frightened modes; falls back to
    // reverse only at a true dead end (all three other directions are walls).
    const Dir rev = opposite(m_dir);

    // Eyes mode: use precomputed BFS distances for true shortest-path navigation
    // so the ghost always takes the optimal route home regardless of maze topology.
    // Other modes: use the classic Manhattan-distance heuristic toward the target tile.
    const bool eyesMode = (m_mode == GhostMode::Eyes);
    const auto [tCol, tRow] = eyesMode ? std::make_pair(0, 0) : targetTile();

    int bestDist = INT_MAX;
    Dir bestDir  = Dir::None;

    for (Dir d : kAllDirs) {
        if (d == rev) continue;
        auto [dc, dr] = dirOffset(d);
        int nc = (m_col + dc + m_wallLayer.cols) % m_wallLayer.cols;
        int nr = m_row + dr;
        if (isWall(nc, nr)) continue;

        int dist;
        if (eyesMode) {
            const int idx = nr * m_wallLayer.cols + nc;
            dist = (m_eyesDist[idx] >= 0) ? m_eyesDist[idx] : INT_MAX;
        } else {
            dist = std::abs(nc - tCol) + std::abs(nr - tRow);
        }

        if (dist < bestDist) {
            bestDist = dist;
            bestDir  = d;
        }
    }

    if (bestDir == Dir::None)
        bestDir = rev;

    return bestDir;
}

void Ghost::reverseDirection() {
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
    if (m_mode == GhostMode::Eyes) return;
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
    if (m_mode == GhostMode::Eyes) return;  // eaten ghosts finish navigating home unaffected
    m_mode     = returnMode;
    m_flashing = false;
    m_animator.setClip("move");
}

void Ghost::startEyes() {
    m_mode     = GhostMode::Eyes;
    m_flashing = false;
    m_animator.setClip("move"); // body hidden in Eyes mode; animator still ticks
}

void Ghost::respawn() {
    m_col    = m_startCol;
    m_row    = m_startRow;
    m_tgtCol = m_startCol;
    m_tgtRow = m_startRow;
    m_x      = m_startCol * kGridSize + kGridSize * 0.5f;
    m_y      = m_startRow * kGridSize + kGridSize * 0.5f;
    m_mode   = GhostMode::Scatter;
    m_flashing = false;
    m_dir    = chooseDirection();
    setTarget(m_dir);
    m_animator.setClip("move");
}

float Ghost::currentSpeed(float normalSpeed, float frightenedSpeed, float eyesSpeed) const {
    switch (m_mode) {
        case GhostMode::Frightened: return frightenedSpeed;
        case GhostMode::Eyes:       return eyesSpeed;
        default:                    return normalSpeed;
    }
}

void Ghost::setTarget(Dir dir) {
    auto [dc, dr] = dirOffset(dir);
    m_tgtCol = (m_col + dc + m_wallLayer.cols) % m_wallLayer.cols;
    m_tgtRow = m_row + dr;
}

void Ghost::update(float dt, int pacCol, int pacRow, Dir pacDir, int blinkyCol, int blinkyRow,
                   float normalSpeed, float frightenedSpeed, float eyesSpeed) {
    m_pacCol    = pacCol;
    m_pacRow    = pacRow;
    m_pacDir    = pacDir;
    m_blinkyCol = blinkyCol;
    m_blinkyRow = blinkyRow;

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
    float step = currentSpeed(normalSpeed, frightenedSpeed, eyesSpeed) * kGridSize * dt;

    if (step >= dist) {
        m_col = m_tgtCol;
        m_row = m_tgtRow;
        m_x   = m_col * kGridSize + kGridSize * 0.5f;
        m_y   = m_row * kGridSize + kGridSize * 0.5f;

        // Eyes: respawn automatically on reaching spawn tile.
        if (m_mode == GhostMode::Eyes && m_col == m_startCol && m_row == m_startRow) {
            respawn();
            return;
        }

        m_dir = chooseDirection();
        setTarget(m_dir);
    } else {
        m_x += (dx / dist) * step;
        m_y += (dy / dist) * step;
    }

    m_animator.update(dt);
}

void Ghost::render(Engine::Renderer2D& renderer, int renderLayer, float offsetY) const {
    const float left = m_x - kRenderSize * 0.5f;
    const float top  = m_y - kRenderSize * 0.5f + offsetY;

    // Body: hidden in Eyes mode (only the face navigates back).
    if (m_mode != GhostMode::Eyes) {
        const auto uv = m_animator.currentFrameUVs();
        renderer.drawTexturedRect(left, top, kRenderSize, kRenderSize,
                                  m_animator.sheet().texture(),
                                  uv.u0, uv.v0, uv.u1, uv.v1,
                                  0.f, {1.f, 1.f, 1.f, 1.f}, renderLayer);
    }

    // Face: drawn centred on the body during normal movement and Eyes mode.
    // Hidden during frightened/flash — the frightened sprite already has a face.
    // Eyes mode uses the row-1 frames (offset by kFaceSheetCols) for better visibility.
    if (m_mode != GhostMode::Frightened && m_faceSheet) {
        const float faceOffset = (kRenderSize - kFaceSize) * 0.5f;
        const int   faceRow    = (m_mode == GhostMode::Eyes) ? kFaceSheetCols : 0;
        const int   faceFrame  = faceRow + static_cast<int>(m_type);
        const auto  faceUV     = m_faceSheet->getFrameUVs(faceFrame);
        renderer.drawTexturedRect(left + faceOffset, top + faceOffset, kFaceSize, kFaceSize,
                                  m_faceSheet->texture(),
                                  faceUV.u0, faceUV.v0, faceUV.u1, faceUV.v1,
                                  0.f, {1.f, 1.f, 1.f, 1.f}, renderLayer);
    }
}
