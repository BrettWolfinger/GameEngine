#pragma once
#include <string_view>
#include <glm/glm.hpp>
#include "Renderer2D.h"

namespace Engine {
namespace PixelFont {

void  drawChar          (Renderer2D& r, char c,           float x,  float y, float s, const glm::vec4& col, int layer = 0);
float drawString        (Renderer2D& r, std::string_view, float x,  float y, float s, const glm::vec4& col, int layer = 0);
void  drawStringCentered(Renderer2D& r, std::string_view, float cx, float y, float s, const glm::vec4& col, int layer = 0);
void  drawStringCentered(Renderer2D& r, int n,            float cx, float y, float s, const glm::vec4& col, int layer = 0);
float stringWidth       (std::string_view text, float s);

} // namespace PixelFont
} // namespace Engine
