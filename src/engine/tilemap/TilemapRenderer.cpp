#include "TilemapRenderer.h"
#include <algorithm>

namespace Engine::Tilemap {

void renderLayer(Renderer2D&        renderer,
                 const TileLayer&   layer,
                 const SpriteSheet& sheet,
                 const TilesetRef&  tileset,
                 float              tileRenderSize,
                 int                rl,
                 float              cameraX,
                 float              cameraY,
                 float              viewportW,
                 float              viewportH,
                 const glm::vec4&   tint) {
    // Column culling
    int firstCol = 0, lastCol = layer.cols;
    if (viewportW > 0.f) {
        firstCol = std::max(0,           static_cast<int>(cameraX / tileRenderSize));
        lastCol  = std::min(layer.cols,  static_cast<int>((cameraX + viewportW) / tileRenderSize) + 1);
    }

    // Row culling
    int firstRow = 0, lastRow = layer.rows;
    if (viewportH > 0.f) {
        firstRow = std::max(0,           static_cast<int>(cameraY / tileRenderSize));
        lastRow  = std::min(layer.rows,  static_cast<int>((cameraY + viewportH) / tileRenderSize) + 1);
    }

    for (int row = firstRow; row < lastRow; ++row) {
        for (int col = firstCol; col < lastCol; ++col) {
            uint32_t raw = layer.gids[row * layer.cols + col];
            if (stripFlips(raw) == 0) continue;

            int localId  = static_cast<int>(stripFlips(raw)) - tileset.firstGid;
            auto flipped = applyFlips(sheet.getFrameUVs(localId), raw);

            renderer.drawTexturedRect(
                col * tileRenderSize - cameraX,
                row * tileRenderSize - cameraY,
                tileRenderSize, tileRenderSize,
                sheet.texture(),
                flipped.uv.u0, flipped.uv.v0, flipped.uv.u1, flipped.uv.v1,
                flipped.angle, tint,
                rl
            );
        }
    }
}

} // namespace Engine::Tilemap
