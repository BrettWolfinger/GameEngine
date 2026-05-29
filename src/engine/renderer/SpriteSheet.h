#pragma once
#include "UVRect.h"
#include "Texture.h"
#include <memory>

namespace Engine {

/// Tag for the frame-pixel SpriteSheet constructor.
struct FrameSize { int w, h; };

class SpriteSheet {
public:
    /// Low-level constructor — prefer SpriteSheet(texture, FrameSize) when frame pixel
    /// dimensions are known, which is almost always.
    SpriteSheet(std::shared_ptr<Texture> texture, int cols, int rows);

    /// Construct by frame pixel dimensions — cols/rows derived from texture size at load time.
    /// Throws if texture dimensions are not exact multiples of frameW/frameH.
    SpriteSheet(std::shared_ptr<Texture> texture, FrameSize frame);

    // Returns UV coordinates for the given zero-based frame index (row-major order).
    UVRect getFrameUVs(int frameIndex) const;

    // Returns UV coordinates spanning wFrames x hFrames cells, anchored at topLeftFrame.
    UVRect getFrameUVs(int topLeftFrame, int wFrames, int hFrames) const;

    const Texture& texture()  const { return *m_texture; }
    int            frameCount() const { return m_cols * m_rows; }

private:
    std::shared_ptr<Texture> m_texture;
    int m_cols;
    int m_rows;
};

} // namespace Engine
