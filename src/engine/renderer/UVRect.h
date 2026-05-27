#pragma once

namespace Engine {

/// Normalized UV coordinates for a rectangular region of a texture.
/// (u0, v0) is the top-left corner; (u1, v1) is the bottom-right corner.
struct UVRect {
    float u0, v0, u1, v1;
};

} // namespace Engine
