#pragma once
#include "Tilemap.h"
#include <functional>

namespace Engine::Tilemap {

/// AABB sweep collider against a static tile grid.
/// The engine handles geometry math; the caller supplies a predicate
/// that decides which tile GIDs are solid.
class Collider {
public:
    using SolidFn = std::function<bool(uint32_t gid)>;

    /// tileW and tileH are in world pixels (native tile size × render scale).
    Collider(const TileLayer& layer, int tileW, int tileH, SolidFn solid);

    struct SweepResult {
        float dx, dy;  // actual displacement to apply
        bool  hitX;    // stopped by a solid tile on the X axis
        bool  hitY;    // stopped by a solid tile on the Y axis
    };

    /// Sweep an AABB at (x, y) with size (w, h) by (dx, dy).
    /// Resolves X then Y independently to avoid corner catching.
    SweepResult sweep(float x, float y, float w, float h, float dx, float dy) const;

    /// Returns true if the tile at the given world position is solid.
    bool isSolidAt(float worldX, float worldY) const;

private:
    bool tileAt(int col, int row) const;

    const TileLayer* m_layer;
    int     m_tileW;
    int     m_tileH;
    SolidFn m_solid;
};

} // namespace Engine::Tilemap
