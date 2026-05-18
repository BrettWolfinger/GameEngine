#pragma once
#include <engine/renderer/Renderer2D.h>
#include <glm/glm.hpp>

// 7-segment glyph renderer — draws digits and letters via Renderer2D::drawRect.
// Segments: a=top  b=top-right  c=bottom-right  d=bottom  e=bottom-left  f=top-left  g=middle
// Each glyph is 3s wide × 5s tall at scale s.

inline void drawDigit(Engine::Renderer2D& r, int d, float x, float y, float s, const glm::vec4& col) {
    static const int segs[10] = { 63, 6, 91, 79, 102, 109, 125, 7, 127, 111 };
    int mask = (d >= 0 && d <= 9) ? segs[d] : 0;
    if (mask &  1) r.drawRect(x,     y,     3*s,  s,  col); // a top
    if (mask &  2) r.drawRect(x+2*s, y,      s,  3*s, col); // b top-right
    if (mask &  4) r.drawRect(x+2*s, y+2*s,  s,  3*s, col); // c bottom-right
    if (mask &  8) r.drawRect(x,     y+4*s, 3*s,  s,  col); // d bottom
    if (mask & 16) r.drawRect(x,     y+2*s,  s,  3*s, col); // e bottom-left
    if (mask & 32) r.drawRect(x,     y,       s,  3*s, col); // f top-left
    if (mask & 64) r.drawRect(x,     y+2*s, 3*s,  s,  col); // g middle
}

// "P" = segments a, b, e, f, g
inline void drawP(Engine::Renderer2D& r, float x, float y, float s, const glm::vec4& col) {
    r.drawRect(x,     y,     3*s,  s,  col); // a top
    r.drawRect(x+2*s, y,      s,  3*s, col); // b top-right
    r.drawRect(x,     y+2*s,  s,  3*s, col); // e bottom-left
    r.drawRect(x,     y,       s,  3*s, col); // f top-left
    r.drawRect(x,     y+2*s, 3*s,  s,  col); // g middle
}

// Draws integer n (up to 2 digits) centered on cx
inline void drawNumber(Engine::Renderer2D& r, int n, float cx, float y, float s, const glm::vec4& col) {
    float digitW = 3*s, gap = s;
    float totalW = (n >= 10) ? digitW + gap + digitW : digitW;
    float x = cx - totalW / 2.f;
    if (n >= 10) { drawDigit(r, n / 10, x, y, s, col); x += digitW + gap; }
    drawDigit(r, n % 10, x, y, s, col);
}

// Draws "NP" (e.g. "1P" or "2P") centered on cx
inline void drawNP(Engine::Renderer2D& r, int n, float cx, float y, float s, const glm::vec4& col) {
    float digitW = 3*s, gap = s;
    float x = cx - (digitW + gap + digitW) / 2.f;
    drawDigit(r, n, x, y, s, col);
    drawP(r, x + digitW + gap, y, s, col);
}
