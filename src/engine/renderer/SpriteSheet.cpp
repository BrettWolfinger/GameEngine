#include "SpriteSheet.h"
#include <stdexcept>

namespace Engine {

SpriteSheet::SpriteSheet(std::shared_ptr<Texture> texture, int cols, int rows)
    : m_texture(std::move(texture)), m_cols(cols), m_rows(rows)
{
    if (!m_texture) throw std::invalid_argument("SpriteSheet: null texture");
    if (cols <= 0 || rows <= 0) throw std::invalid_argument("SpriteSheet: cols/rows must be > 0");
}

SpriteSheet::SpriteSheet(std::shared_ptr<Texture> texture, FrameSize frame)
    : m_texture(std::move(texture))
    , m_cols(m_texture->getWidth()  / frame.w)
    , m_rows(m_texture->getHeight() / frame.h)
{
    if (!m_texture) throw std::invalid_argument("SpriteSheet: null texture");
    if (frame.w <= 0 || frame.h <= 0) throw std::invalid_argument("SpriteSheet: frame size must be > 0");
    if (m_texture->getWidth()  % frame.w != 0 ||
        m_texture->getHeight() % frame.h != 0)
        throw std::invalid_argument("SpriteSheet: texture dimensions not divisible by frame size");
}

UVRect SpriteSheet::getFrameUVs(int frameIndex) const {
    // Row-major layout: frame 0 = top-left, frame (cols-1) = top-right of first row
    const int col = frameIndex % m_cols;
    const int row = frameIndex / m_cols;

    const float fw = 1.f / static_cast<float>(m_cols);
    const float fh = 1.f / static_cast<float>(m_rows);

    return UVRect{
        col       * fw,   // u0
        row       * fh,   // v0
        (col + 1) * fw,   // u1
        (row + 1) * fh,   // v1
    };
}

UVRect SpriteSheet::getFrameUVs(int topLeftFrame, int wFrames, int hFrames) const {
    UVRect tl = getFrameUVs(topLeftFrame);
    int brCol = (topLeftFrame % m_cols) + wFrames - 1;
    int brRow = (topLeftFrame / m_cols) + hFrames - 1;
    UVRect br = getFrameUVs(brRow * m_cols + brCol);
    return { tl.u0, tl.v0, br.u1, br.v1 };
}

} // namespace Engine
