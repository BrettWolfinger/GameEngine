#include "SegmentFont.h"
#include <string>
#include <cctype>

namespace Engine {
namespace SegmentFont {

// Segment bitmask: a=1(top) b=2(top-right) c=4(bottom-right) d=8(bottom) e=16(bottom-left) f=32(top-left) g=64(middle)
static const int s_digits[10] = { 63, 6, 91, 79, 102, 109, 125, 7, 127, 111 };
static const int s_alpha[26]  = {
    119, // A  a+b+c+e+f+g
    124, // B  c+d+e+f+g     (lowercase b approximation)
     57, // C  a+d+e+f
     94, // D  b+c+d+e+g     (lowercase d approximation)
    121, // E  a+d+e+f+g
    113, // F  a+e+f+g
     61, // G  a+c+d+e+f
    118, // H  b+c+e+f+g
     48, // I  e+f            (left-side approximation)
     30, // J  b+c+d+e
    117, // K  a+c+e+f+g     (approximate)
     56, // L  d+e+f
     55, // M  a+b+c+e+f     (approximate — missing crossbars)
     55, // N  a+b+c+e+f     (top + four verticals, no bottom or middle)
     63, // O  a+b+c+d+e+f   (same as 0)
    115, // P  a+b+e+f+g
    103, // Q  a+b+c+f+g     (approximate)
     80, // R  e+g            (lowercase r)
    109, // S  a+c+d+f+g     (same as 5)
     49, // T  a+e+f          (approximate — top bar + left stem)
     62, // U  b+c+d+e+f
     28, // V  c+d+e          (approximate)
     62, // W  b+c+d+e+f     (same as U, approximate)
    118, // X  b+c+e+f+g     (same as H, approximate)
    110, // Y  b+c+d+f+g
     91, // Z  a+b+d+e+g     (same as 2)
};

static void drawSegments(Renderer2D& r, int mask, float x, float y, float s, const glm::vec4& col, int layer) {
    if (mask &  1) r.drawRect(x,       y,     3*s,  s,  col, layer); // a top
    if (mask &  2) r.drawRect(x + 2*s, y,      s,  3*s, col, layer); // b top-right
    if (mask &  4) r.drawRect(x + 2*s, y+2*s,  s,  3*s, col, layer); // c bottom-right
    if (mask &  8) r.drawRect(x,       y+4*s, 3*s,  s,  col, layer); // d bottom
    if (mask & 16) r.drawRect(x,       y+2*s,  s,  3*s, col, layer); // e bottom-left
    if (mask & 32) r.drawRect(x,       y,       s,  3*s, col, layer); // f top-left
    if (mask & 64) r.drawRect(x,       y+2*s, 3*s,  s,  col, layer); // g middle
}

void drawChar(Renderer2D& r, char c, float x, float y, float s, const glm::vec4& col, int layer) {
    c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    int mask = 0;
    if      (c >= '0' && c <= '9') mask = s_digits[c - '0'];
    else if (c >= 'A' && c <= 'Z') mask = s_alpha[c - 'A'];
    else                           return; // space or unsupported — no-op
    drawSegments(r, mask, x, y, s, col, layer);
}

float stringWidth(std::string_view text, float s) {
    if (text.empty()) return 0.f;
    // N glyphs (3s each) + (N-1) gaps (s each)
    return s * (4.f * static_cast<float>(text.size()) - 1.f);
}

float drawString(Renderer2D& r, std::string_view text, float x, float y, float s, const glm::vec4& col, int layer) {
    float advance = 4.f * s; // glyph width (3s) + gap (s)
    for (char c : text) {
        drawChar(r, c, x, y, s, col, layer);
        x += advance;
    }
    return stringWidth(text, s);
}

void drawStringCentered(Renderer2D& r, std::string_view text, float cx, float y, float s, const glm::vec4& col, int layer) {
    drawString(r, text, cx - stringWidth(text, s) * 0.5f, y, s, col, layer);
}

void drawStringCentered(Renderer2D& r, int n, float cx, float y, float s, const glm::vec4& col, int layer) {
    drawStringCentered(r, std::to_string(n), cx, y, s, col, layer);
}

} // namespace SegmentFont
} // namespace Engine
