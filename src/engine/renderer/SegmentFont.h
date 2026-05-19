#pragma once
#include <string_view>
#include <glm/glm.hpp>
#include "Renderer2D.h"

// 7-segment glyph font. Each glyph is (3*s) wide x (5*s) tall.
// (x, y) is top-left; Y increases downward.
// Input is automatically uppercased. Unsupported characters (including space) advance the cursor without drawing.
// Segment bitmask: a=1(top) b=2(top-right) c=4(bottom-right) d=8(bottom) e=16(bottom-left) f=32(top-left) g=64(middle)

namespace Engine {
namespace SegmentFont {

void  drawChar          (Renderer2D& r, char c,           float x,  float y, float s, const glm::vec4& col);
float drawString        (Renderer2D& r, std::string_view, float x,  float y, float s, const glm::vec4& col); // returns rendered width
void  drawStringCentered(Renderer2D& r, std::string_view, float cx, float y, float s, const glm::vec4& col);
void  drawStringCentered(Renderer2D& r, int n,            float cx, float y, float s, const glm::vec4& col);
float stringWidth       (std::string_view text, float s);

} // namespace SegmentFont
} // namespace Engine
