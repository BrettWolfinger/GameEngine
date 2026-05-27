#include "Pacman.h"
#include <engine/core/Input.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <cstdlib>

static constexpr float kSpeed   = 7.5f;
static constexpr float kHalfPi  = 1.5707963268f;

// Grid tile size in pixels — used for all position and movement math.
static constexpr float kGridSize = static_cast<float>(TILE * SCALE);

// Rendered sprite size in pixels — may differ from kGridSize.
static constexpr float kRenderSize = static_cast<float>(kPacFrameSize * SCALE);

static std::pair<int,int> dirOffset(Dir d) {
    switch (d) {
        case Dir::Up:    return { 0, -1};
        case Dir::Down:  return { 0,  1};
        case Dir::Left:  return {-1,  0};
        case Dir::Right: return { 1,  0};
        default:         return { 0,  0};
    }
}

Pacman::Pacman(const Engine::Tilemap::TileLayer& wallLayer,
               std::shared_ptr<Engine::SpriteSheet> sheet,
               int startCol, int startRow)
    : m_wallLayer(wallLayer)
    , m_animator(sheet)
{
    m_animator.addClip("move", { {0, 1, 2, 3}, 0.1f, Engine::PlayMode::Loop });

    // Death: 8 frames across rows 1 and 2 (frame indices 4–11)
    m_animator.addClip("death", {
        { kPacSheetCols*1+0, kPacSheetCols*1+1, kPacSheetCols*1+2, kPacSheetCols*1+3,
          kPacSheetCols*2+0, kPacSheetCols*2+1, kPacSheetCols*2+2, kPacSheetCols*2+3 },
        0.15f, Engine::PlayMode::OneShot
    });

    m_animator.setClip("move");

    m_startCol = startCol;
    m_startRow = startRow;
    m_col      = m_tgtCol = startCol;
    m_row      = m_tgtRow = startRow;
    m_x = m_col * kGridSize + kGridSize * 0.5f;
    m_y = m_row * kGridSize + kGridSize * 0.5f;
}

bool Pacman::isWall(int col, int row) const {
    if (row < 0 || row >= m_wallLayer.rows) return true;
    col = (col + m_wallLayer.cols) % m_wallLayer.cols;
    return Engine::Tilemap::stripFlips(m_wallLayer.gids[row * m_wallLayer.cols + col]) != 0;
}

bool Pacman::canMove(int col, int row, Dir dir) const {
    auto [dc, dr] = dirOffset(dir);
    return !isWall(col + dc, row + dr);
}

void Pacman::setTarget(int fromCol, int fromRow, Dir dir) {
    auto [dc, dr] = dirOffset(dir);
    m_tgtCol = (fromCol + dc + m_wallLayer.cols) % m_wallLayer.cols;
    m_tgtRow = fromRow + dr;
}

void Pacman::startDeath() {
    m_dying   = true;
    m_dir     = Dir::None;
    m_nextDir = Dir::None;
    m_animator.setClip("death");
}

bool Pacman::isDeathDone() const {
    return m_dying && m_animator.isFinished();
}

void Pacman::respawn() {
    m_col    = m_tgtCol = m_startCol;
    m_row    = m_tgtRow = m_startRow;
    m_x      = m_startCol * kGridSize + kGridSize * 0.5f;
    m_y      = m_startRow * kGridSize + kGridSize * 0.5f;
    m_dir    = Dir::None;
    m_nextDir = Dir::None;
    m_dying  = false;
    m_animator.setClip("move");
}

void Pacman::update(float dt) {
    // During death animation only advance the animator — no input or movement.
    if (m_dying) {
        m_animator.update(dt);
        return;
    }

    // --- input ---
    if (Engine::Input::isKeyDown(GLFW_KEY_UP))    m_nextDir = Dir::Up;
    if (Engine::Input::isKeyDown(GLFW_KEY_DOWN))  m_nextDir = Dir::Down;
    if (Engine::Input::isKeyDown(GLFW_KEY_LEFT))  m_nextDir = Dir::Left;
    if (Engine::Input::isKeyDown(GLFW_KEY_RIGHT)) m_nextDir = Dir::Right;

    // --- start moving if stationary ---
    if (m_dir == Dir::None) {
        if (m_nextDir != Dir::None && canMove(m_col, m_row, m_nextDir)) {
            m_dir = m_nextDir;
            setTarget(m_col, m_row, m_dir);
        }
        return;
    }

    // --- advance toward target cell center ---
    // Adjust target x for tunnel wrap so the sprite exits one side and enters the other,
    // rather than sliding backwards across the screen.
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
    float step = kSpeed * kGridSize * dt;

    if (step >= dist) {
        m_col = m_tgtCol;
        m_row = m_tgtRow;
        m_x   = m_col * kGridSize + kGridSize * 0.5f; // snap to actual on-screen position
        m_y   = m_row * kGridSize + kGridSize * 0.5f;

        if (m_nextDir != Dir::None && canMove(m_col, m_row, m_nextDir)) {
            m_dir = m_nextDir;
            setTarget(m_col, m_row, m_dir);
        } else if (canMove(m_col, m_row, m_dir)) {
            setTarget(m_col, m_row, m_dir);
        } else {
            m_dir = Dir::None;
        }
    } else {
        m_x += (dx / dist) * step;
        m_y += (dy / dist) * step;
    }

    m_animator.update(dt);
}

void Pacman::render(Engine::Renderer2D& renderer, int renderLayer) const {
    auto  uv    = m_animator.currentFrameUVs();
    float angle = 0.f;

    // Death animation is directional-agnostic — render without any transform.
    if (!m_dying) {
        switch (m_dir == Dir::None ? m_nextDir : m_dir) {
            case Dir::Left:  std::swap(uv.u0, uv.u1); break;
            case Dir::Up:    angle = -kHalfPi;         break;
            case Dir::Down:  angle = +kHalfPi;         break;
            default: break;
        }
    }

    renderer.drawTexturedRect(
        m_x - kRenderSize * 0.5f, m_y - kRenderSize * 0.5f,
        kRenderSize, kRenderSize,
        m_animator.sheet().texture(),
        uv.u0, uv.v0, uv.u1, uv.v1,
        angle, {1.f, 1.f, 1.f, 1.f},
        renderLayer
    );
}
