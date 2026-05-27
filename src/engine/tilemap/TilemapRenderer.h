#pragma once
#include <glm/glm.hpp>
#include "Tilemap.h"
#include "../renderer/Renderer2D.h"
#include "../renderer/SpriteSheet.h"

namespace Engine::Tilemap {

/// Draw all non-empty tiles in a layer.
///
/// cameraX/Y offset the world into screen space. viewportW/H enable
/// column/row culling — pass 0 to disable culling and render the full layer.
void renderLayer(Renderer2D&        renderer,
                 const TileLayer&   layer,
                 const SpriteSheet& sheet,
                 const TilesetRef&  tileset,
                 float              tileRenderSize,
                 int                renderLayer = 0,
                 float              cameraX     = 0.f,
                 float              cameraY     = 0.f,
                 float              viewportW   = 0.f,
                 float              viewportH   = 0.f,
                 const glm::vec4&   tint        = { 1.f, 1.f, 1.f, 1.f });

} // namespace Engine::Tilemap
