#include "Ghost.h"
#include <cmath>
#include <climits>

static constexpr float kGhostSpeed  = 6.0f; // tiles per second
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
{
    // Each ghost type occupies one row; columns are animation frames.
    const int base = static_cast<int>(m_type) * kGhostSheetCols;
    m_animator.addClip("move", { {base, base+1, base+2, base+3}, 0.15f, Engine::PlayMode::Loop });
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

Dir Ghost::chooseDirection() const {
    // Classic Pac-Man tie-break priority: Up, Left, Down, Right.
    static constexpr Dir kPriority[] = { Dir::Up, Dir::Left, Dir::Down, Dir::Right };

    const Dir rev = opposite(m_dir); // direction ghosts may not reverse into
    auto [tCol, tRow] = scatterCorner();

    int bestDist = INT_MAX;
    Dir bestDir  = Dir::None;

    for (Dir d : kPriority) {
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

void Ghost::setTarget(Dir dir) {
    auto [dc, dr] = dirOffset(dir);
    m_tgtCol = (m_col + dc + m_wallLayer.cols) % m_wallLayer.cols;
    m_tgtRow = m_row + dr;
}

void Ghost::update(float dt) {
    float tx   = m_tgtCol * kGridSize + kGridSize * 0.5f;
    float ty   = m_tgtRow * kGridSize + kGridSize * 0.5f;
    float dx   = tx - m_x;
    float dy   = ty - m_y;
    float dist = std::abs(dx) + std::abs(dy);
    float step = kGhostSpeed * kGridSize * dt;

    if (step >= dist) {
        m_x   = tx;
        m_y   = ty;
        m_col = m_tgtCol;
        m_row = m_tgtRow;

        m_dir = chooseDirection();
        setTarget(m_dir);
    } else {
        m_x += (dx / dist) * step;
        m_y += (dy / dist) * step;
    }

    m_animator.update(dt);
}

void Ghost::render(Engine::Renderer2D& renderer, int renderLayer) const {
    const auto uv = m_animator.currentFrameUVs();

    renderer.drawTexturedRect(
        m_x - kRenderSize * 0.5f, m_y - kRenderSize * 0.5f,
        kRenderSize, kRenderSize,
        m_animator.sheet().texture(),
        uv.u0, uv.v0, uv.u1, uv.v1,
        0.f, {1.f, 1.f, 1.f, 1.f},
        renderLayer
    );
}
