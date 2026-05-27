#include "PixelFont.h"
#include <string>
#include <cctype>
#include <cstdint>

namespace Engine {
namespace PixelFont {

static const uint8_t s_digits[10][7] = {
    { 14, 17, 17, 17, 17, 17, 14 }, // 0
    {  4, 12,  4,  4,  4,  4, 14 }, // 1
    { 14, 17,  1,  2,  4,  8, 31 }, // 2
    { 14, 17,  1,  6,  1, 17, 14 }, // 3
    {  2,  6, 10, 18, 31,  2,  2 }, // 4
    { 31, 16, 16, 30,  1,  1, 30 }, // 5
    { 14, 16, 16, 30, 17, 17, 14 }, // 6
    { 31,  1,  2,  4,  4,  4,  4 }, // 7
    { 14, 17, 17, 14, 17, 17, 14 }, // 8
    { 14, 17, 17, 15,  1, 17, 14 }, // 9
};

static const uint8_t s_alpha[26][7] = {
    { 14, 17, 17, 31, 17, 17, 17 }, // A
    { 30, 17, 17, 30, 17, 17, 30 }, // B
    { 14, 17, 16, 16, 16, 17, 14 }, // C
    { 28, 18, 17, 17, 17, 18, 28 }, // D
    { 31, 16, 16, 30, 16, 16, 31 }, // E
    { 31, 16, 16, 30, 16, 16, 16 }, // F
    { 14, 17, 16, 23, 17, 17, 15 }, // G
    { 17, 17, 17, 31, 17, 17, 17 }, // H
    { 14,  4,  4,  4,  4,  4, 14 }, // I
    {  7,  2,  2,  2,  2, 18, 12 }, // J
    { 17, 18, 20, 24, 20, 18, 17 }, // K
    { 16, 16, 16, 16, 16, 16, 31 }, // L
    { 17, 27, 21, 17, 17, 17, 17 }, // M
    { 17, 25, 21, 19, 17, 17, 17 }, // N
    { 14, 17, 17, 17, 17, 17, 14 }, // O
    { 30, 17, 17, 30, 16, 16, 16 }, // P
    { 14, 17, 17, 17, 21, 18, 13 }, // Q
    { 30, 17, 17, 30, 20, 18, 17 }, // R
    { 15, 16, 16, 14,  1,  1, 30 }, // S
    { 31,  4,  4,  4,  4,  4,  4 }, // T
    { 17, 17, 17, 17, 17, 17, 14 }, // U
    { 17, 17, 17, 17, 17, 10,  4 }, // V
    { 17, 17, 17, 21, 21, 27, 17 }, // W
    { 17, 17, 10,  4, 10, 17, 17 }, // X
    { 17, 17, 10,  4,  4,  4,  4 }, // Y
    { 31,  1,  2,  4,  8, 16, 31 }, // Z
};

void drawChar(Renderer2D& r, char c, float x, float y, float s, const glm::vec4& col, int layer) {
    c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    const uint8_t* rows = nullptr;
    if      (c >= '0' && c <= '9') rows = s_digits[c - '0'];
    else if (c >= 'A' && c <= 'Z') rows = s_alpha[c - 'A'];
    else                           return;

    for (int row = 0; row < 7; ++row) {
        uint8_t bits = rows[row];
        for (int px = 0; px < 5; ++px) {
            if (bits & (1 << (4 - px)))
                r.drawRect(x + px * s, y + row * s, s, s, col, layer);
        }
    }
}

float stringWidth(std::string_view text, float s) {
    if (text.empty()) return 0.f;
    return s * (6.f * static_cast<float>(text.size()) - 1.f);
}

float drawString(Renderer2D& r, std::string_view text, float x, float y, float s, const glm::vec4& col, int layer) {
    float advance = 6.f * s;
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

} // namespace PixelFont
} // namespace Engine
