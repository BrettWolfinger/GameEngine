#include "TilemapCollider.h"
#include <cmath>
#include <algorithm>

namespace Engine::Tilemap {

Collider::Collider(const TileLayer& layer, int tileW, int tileH, SolidFn solid)
    : m_layer(&layer), m_tileW(tileW), m_tileH(tileH), m_solid(std::move(solid))
{}

bool Collider::tileAt(int col, int row) const {
    if (col < 0 || row < 0 || col >= m_layer->cols || row >= m_layer->rows)
        return false;
    const uint32_t gid = stripFlips(m_layer->gids[row * m_layer->cols + col]);
    return m_solid(gid);
}

bool Collider::isSolidAt(float worldX, float worldY) const {
    return tileAt(static_cast<int>(std::floor(worldX / m_tileW)),
                  static_cast<int>(std::floor(worldY / m_tileH)));
}

Collider::SweepResult Collider::sweep(
    float x, float y, float w, float h, float dx, float dy) const
{
    SweepResult result { dx, dy, false, false };

    // --- X axis ---
    if (dx != 0.f) {
        const int topRow    = static_cast<int>(std::floor(y / m_tileH));
        const int bottomRow = static_cast<int>(std::ceil((y + h) / m_tileH)) - 1;

        if (dx > 0.f) {
            const int col = static_cast<int>(std::floor((x + w + dx) / m_tileW));
            for (int row = topRow; row <= bottomRow; ++row) {
                if (tileAt(col, row)) {
                    result.dx = static_cast<float>(col * m_tileW) - (x + w);
                    result.hitX = true;
                    break;
                }
            }
        } else {
            const int col = static_cast<int>(std::floor((x + dx) / m_tileW));
            for (int row = topRow; row <= bottomRow; ++row) {
                if (tileAt(col, row)) {
                    result.dx = static_cast<float>((col + 1) * m_tileW) - x;
                    result.hitX = true;
                    break;
                }
            }
        }
    }

    // --- Y axis (uses X-resolved position) ---
    if (dy != 0.f) {
        const float resolvedX = x + result.dx;
        const int leftCol  = static_cast<int>(std::floor(resolvedX / m_tileW));
        const int rightCol = static_cast<int>(std::ceil((resolvedX + w) / m_tileW)) - 1;

        if (dy > 0.f) {
            const int row = static_cast<int>(std::floor((y + h + dy) / m_tileH));
            for (int col = leftCol; col <= rightCol; ++col) {
                if (tileAt(col, row)) {
                    result.dy = static_cast<float>(row * m_tileH) - (y + h);
                    result.hitY = true;
                    break;
                }
            }
        } else {
            const int row = static_cast<int>(std::floor((y + dy) / m_tileH));
            for (int col = leftCol; col <= rightCol; ++col) {
                if (tileAt(col, row)) {
                    result.dy = static_cast<float>((row + 1) * m_tileH) - y;
                    result.hitY = true;
                    break;
                }
            }
        }
    }

    return result;
}

} // namespace Engine::Tilemap
