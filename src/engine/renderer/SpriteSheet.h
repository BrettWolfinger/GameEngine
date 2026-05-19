#pragma once
#include "Texture.h"
#include <memory>

namespace Engine {

struct UVRect {
    float u0, v0, u1, v1;
};

class SpriteSheet {
public:
    SpriteSheet(std::shared_ptr<Texture> texture, int cols, int rows);

    // Returns UV coordinates for the given zero-based frame index (row-major order).
    UVRect getFrameUVs(int frameIndex) const;

    // Returns UV coordinates spanning widthInCells cells horizontally from frameIndex.
    UVRect getSpanUVs(int frameIndex, int widthInCells) const;

    const Texture& texture()  const { return *m_texture; }
    int            frameCount() const { return m_cols * m_rows; }

private:
    std::shared_ptr<Texture> m_texture;
    int m_cols;
    int m_rows;
};

} // namespace Engine
