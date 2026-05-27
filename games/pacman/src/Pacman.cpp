#include "Pacman.h"
#include <engine/core/Input.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <cstdlib> // std::abs

static constexpr float kSpeed = 7.5f; // tiles per second

static std::pair<int,int> dirOffset(Dir d) {
    switch (d) {
        case Dir::Up:    return { 0, -1};
        case Dir::Down:  return { 0,  1};
        case Dir::Left:  return {-1,  0};
        case Dir::Right: return { 1,  0};
        default:         return { 0,  0};
    }
}

Pacman::Pacman(const Engine::Tilemap::TileLayer& wallLayer)
    : m_wallLayer(wallLayer)
{
    const float tileSize = static_cast<float>(TILE * SCALE);
    m_x = m_col * tileSize + tileSize * 0.5f;
    m_y = m_row * tileSize + tileSize * 0.5f;
}

bool Pacman::isWall(int col, int row) const {
    if (row < 0 || row >= m_wallLayer.rows) return true;
    col = (col + m_wallLayer.cols) % m_wallLayer.cols; // horizontal wrap
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

void Pacman::update(float dt) {
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

    // --- advance toward target cell ---
    const float tileSize = static_cast<float>(TILE * SCALE);
    float tx   = m_tgtCol * tileSize + tileSize * 0.5f;
    float ty   = m_tgtRow * tileSize + tileSize * 0.5f;
    float dx   = tx - m_x;
    float dy   = ty - m_y;
    float dist = std::abs(dx) + std::abs(dy);
    float step = kSpeed * tileSize * dt;

    if (step >= dist) {
        // Arrived — snap to target cell center
        m_x   = tx;
        m_y   = ty;
        m_col = m_tgtCol;
        m_row = m_tgtRow;

        // Try buffered direction, then continue, then stop
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
}

void Pacman::render(Engine::Renderer2D& renderer, int renderLayer) const {
    const float tileSize = static_cast<float>(TILE * SCALE);
    renderer.drawRect(
        m_x - tileSize * 0.5f, m_y - tileSize * 0.5f,
        tileSize, tileSize,
        {1.f, 1.f, 0.f, 1.f}, // yellow placeholder
        renderLayer
    );
}
